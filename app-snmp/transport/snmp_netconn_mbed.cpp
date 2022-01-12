/*
 * Copyright (c) 2021, Nuvoton Technology Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "lwip/apps/snmp_opts.h"

#if LWIP_SNMP && SNMP_USE_NETCONN

#include <string.h>
#include "lwip/api.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "snmp_msg.h"
#include "lwip/sys.h"
#include "lwip/prot/iana.h"
#include "snmp_agent_config.h"

/* Mbed includes */
#include "mbed.h"
#include "mbed_trace.h"

#if MBED_CONF_MBED_TRACE_ENABLE
#define TRACE_GROUP     "lwIP"
#endif

/* Maximum UDP payload size */
#define UDP_PAYLOAD_MAXSIZE         1500

/* Change trap destination IP to SNMP request source
 *
 * Usually, trap destination must be configured separately. If it is configured
 * to dummy defined below, we change the SNMP request source for development.
 *
 * Here, we use reserved IP address for documentation as dummy. Refer to:
 * https://en.wikipedia.org/wiki/Reserved_IP_addresses
 */
#ifdef MYSNMPAGENT_TRAP_DST_IP
#if LWIP_IPV4
#define TRAP_DST_IP_DUMMY               IPADDR4_INIT_BYTES(192, 0, 2, 0)
#else
#define TRAP_DST_IP_DUMMY               IPADDR6_INIT_HOST(0x20010db8, 0xffffffff, 0xffffffff, 0xffffffff)
#endif /* LWIP_IPV6 */
#endif

/* SNMP transport context */
struct SNMPTransportContext
{
    /* Separate socket for sending traps
     *
     * Some network interface e.g. ESP8266 has limited support on "sendto" for
     * sending to different destinations (response and trap) via the same socket.
     * To get around the subtle issue, we create a separate socket for sending
     * traps.
     */
    /* Receive SNMP request and send SNMP response */
    UDPSocket   udpsock;
    /* Send SNMP trap */
    UDPSocket   udpsock_trap;
    uint8_t     udp_payload_buf[UDP_PAYLOAD_MAXSIZE];
#ifdef MYSNMPAGENT_TRAP_DST_IP
    ip_addr_t   ipaddr_trap_dst_redirect;
#endif
};

/** SNMP netconn API worker thread */
static void
snmp_netconn_thread(void *arg)
{
    tr_info("SNMP Agent thread started");

    LWIP_UNUSED_ARG(arg);

    nsapi_size_or_error_t rc_nsapi = NSAPI_ERROR_OK;
    SNMPTransportContext *snmp_trans_ctx = NULL;
    struct pbuf *p = NULL;

    snmp_trans_ctx = new SNMPTransportContext;

    /* Open a network socket on the network stack of the given network interface */
    rc_nsapi = snmp_trans_ctx->udpsock.open(NetworkInterface::get_default_instance());
    if (rc_nsapi != NSAPI_ERROR_OK) {
        tr_err("Open UDP socket on network stack failed: %d", rc_nsapi);
        goto cleanup;
    }

    /* Same as above, but for sending traps */
    rc_nsapi = snmp_trans_ctx->udpsock_trap.open(NetworkInterface::get_default_instance());
    if (rc_nsapi != NSAPI_ERROR_OK) {
        tr_err("Open UDP socket (for sending traps) on network stack failed: %d", rc_nsapi);
        goto cleanup;
    }

    /* Bind to SNMP port */
    rc_nsapi = snmp_trans_ctx->udpsock.bind(LWIP_IANA_PORT_SNMP);
    if (rc_nsapi != NSAPI_ERROR_OK) {
        tr_error("Bind to SNMP port failed: %d", rc_nsapi);
        goto cleanup;
    }

    /* Different than above, unnecessary for sending traps */

    /* We just have one local IP address, which is used for communicating both
     * SNMP response and trap. */
    snmp_traps_handle = snmp_trans_ctx;

    do {
        if (p != NULL) {
            pbuf_free(p);
            p = NULL;
        }

        SocketAddress sockaddr;
        rc_nsapi = snmp_trans_ctx->udpsock.recvfrom(&sockaddr,
                                                    snmp_trans_ctx->udp_payload_buf,
                                                    UDP_PAYLOAD_MAXSIZE);
        if (rc_nsapi <= 0) {
            tr_warn("Receive SNMP request over UDP failed: %d", rc_nsapi);
            continue;
        }

        tr_info("Receive SNMP request (%d) over UDP from: %s:%d",
                rc_nsapi,
                sockaddr.get_ip_address(),
                sockaddr.get_port());

        /* pbuf's tot_len same as UDP payload size
         *
         * pbuf's tot_len means both maximum size and effective size. We should
         * allocate pbuf whose size is exactly the same as (not larger than) UDP
         * payload size, or we are required to call pbuf_realloc() to shrink it.
         */
        p = pbuf_alloc(PBUF_TRANSPORT, rc_nsapi, PBUF_RAM);
        if (p == NULL) {
            tr_error("pbuf_alloc() for SNMP request failed");
            continue;
        }

        /* Copy received SNMP request to pbuf */
        err_t rc_lwip = pbuf_take(p, snmp_trans_ctx->udp_payload_buf, rc_nsapi);
        if (rc_lwip != ERR_OK) {
            tr_error("pbuf_take() for copying SNMP request to pbuf failed: %d", rc_lwip);
            continue;
        }

        ip_addr_t ipaddr;
        ipaddr_aton(sockaddr.get_ip_address(), &ipaddr);

#ifdef MYSNMPAGENT_TRAP_DST_IP
        /* Keep SNMP request source as trap destination when it is dummy */
        ip_addr_copy(snmp_trans_ctx->ipaddr_trap_dst_redirect, ipaddr);
#endif

        /* Pass to SNMP core for handling */
        snmp_receive(snmp_trans_ctx, p, &ipaddr, sockaddr.get_port());

    } while (1);

    /* Clean up resource */
cleanup:

    if (p != NULL) {
        pbuf_free(p);
        p = NULL;
    }

    delete snmp_trans_ctx;
    snmp_trans_ctx = NULL;
}

err_t
snmp_sendto(void *handle, struct pbuf *p, const ip_addr_t *dst, u16_t port)
{
    SNMPTransportContext *snmp_trans_ctx = (SNMPTransportContext *) handle;
    if (snmp_trans_ctx == NULL) {
        tr_error("snmp_sendto(): Invalid handle");
        return ERR_ARG;
    }

    size_t udp_payload_size = p->tot_len;
    void *udp_payload = pbuf_get_contiguous(p,
                                            snmp_trans_ctx->udp_payload_buf,
                                            UDP_PAYLOAD_MAXSIZE,
                                            udp_payload_size,
                                            0);
    if (udp_payload == NULL) {
        tr_error("snmp_sendto(): pbuf_get_contiguous() returns null");
        return ERR_BUF;
    }

#ifdef MYSNMPAGENT_TRAP_DST_IP
    /* For development, if destination IP is dummy, regard it as trap destination
     * IP and change to the SNMP request source. */
    ip_addr_t trap_dst_ip_dummy = TRAP_DST_IP_DUMMY;
    if (ip_addr_cmp(dst, &trap_dst_ip_dummy)) {
        dst = &snmp_trans_ctx->ipaddr_trap_dst_redirect;
        tr_warn("Trap destination IP is dummy. Change to the SNMP request source: %s", ipaddr_ntoa(dst));
    }
#endif

    SocketAddress sockaddr;    
    if (!sockaddr.set_ip_address(ipaddr_ntoa(dst))) {
        tr_error("snmp_sendto(): Invalid destination IP address");
        return ERR_ARG;
    }
    sockaddr.set_port(port);

    /* Send response or trap? */
    bool send_trap = (sockaddr.get_port() == LWIP_IANA_PORT_SNMP_TRAP);

    tr_info("Send SNMP %s (%d) over UDP to: %s:%d",
            send_trap ? "trap" : "response",
            udp_payload_size,
            sockaddr.get_ip_address(),
            sockaddr.get_port());

    UDPSocket &udpsock = send_trap ? snmp_trans_ctx->udpsock_trap : snmp_trans_ctx->udpsock;
    nsapi_size_or_error_t rc_nsapi = udpsock.sendto(sockaddr,
                                                    udp_payload,
                                                    udp_payload_size);
    if (rc_nsapi <= 0) {
        tr_error("Send SNMP %s over UDP failed: %d",
                 send_trap ? "trap" : "response",
                 rc_nsapi);
        return ERR_IF;
    } else if (rc_nsapi != udp_payload_size) {
        tr_error("Send SNMP %s over UDP incomplete: Expected %d but %d",
                 send_trap ? "trap" : "response",
                 udp_payload_size,
                 rc_nsapi);
        return ERR_IF;
    }

    return ERR_OK;
}

u8_t
snmp_get_local_ip_for_dst(void *handle, const ip_addr_t *dst, ip_addr_t *result)
{
    LWIP_UNUSED_ARG(handle);

    /* Use the IP binding to the network interface */
    LWIP_UNUSED_ARG(dst);

    auto net = NetworkInterface::get_default_instance();
    if (net == NULL) {
        tr_err("snmp_get_local_ip_for_dst: Null network interface");
        return 0;
    }

    SocketAddress sockaddr;
    nsapi_size_or_error_t rc_nsapi = net->get_ip_address(&sockaddr);
    if (rc_nsapi != NSAPI_ERROR_OK) {
        tr_err("snmp_get_local_ip_for_dst: Get local IP address failed: %d", rc_nsapi);
        return 0;
    }

    return ipaddr_aton(sockaddr.get_ip_address(), result);
}

/**
 * Starts SNMP Agent.
 */
void
snmp_init(void)
{
  LWIP_ASSERT_CORE_LOCKED();
  sys_thread_new("snmp_netconn", snmp_netconn_thread, NULL, SNMP_STACK_SIZE, SNMP_THREAD_PRIO);
}

#endif /* LWIP_SNMP && SNMP_USE_NETCONN */
