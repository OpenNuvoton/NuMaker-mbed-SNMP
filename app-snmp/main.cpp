/*
 * Copyright (c) 2021, Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Mbed includes */
#include "mbed.h"
#include "mbed_trace.h"

#if MBED_CONF_MBED_TRACE_ENABLE
#define TRACE_GROUP     "Main"
#endif

/* lwIP includes */
#include "lwipstack/LWIPStack.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_mib2.h"
#include "snmp_agent_config.h"

/* lwIP default MIB-2 + private gpio peripheral MIB */
extern "C" const struct snmp_mib gpio_perif_mib;
static const struct snmp_mib *mysnmpagent_mibs[] = {&mib2, &gpio_perif_mib};

/* SNMP device enterprise OID */
static const struct snmp_obj_id mysnmpagent_device_enterprise_oid = {7, {1, 3, 6, 1, 4, 1, MYSNMPAGENT_VENDOR_ENTERPRISE_OID}};

#ifdef MYSNMPAGENT_TRAP_DST_IP
/* SNMP trap destination IP */
static const char mysnmpagent_trap_dst_ip[]     = MYSNMPAGENT_TRAP_DST_IP;
#endif

/* SNMP MIB-2 system group 1.3.6.1.2.1.1 */
static const u8_t mysnmpagent_sysdescr[]        = MYSNMPAGENT_MIB2_SYSDESCR;
static const u16_t mysnmpagent_sysdescr_len     = sizeof(MYSNMPAGENT_MIB2_SYSDESCR) - 1;
static u8_t mysnmpagent_syscontact[256]         = MYSNMPAGENT_MIB2_SYSCONTACT;
static u16_t mysnmpagent_syscontact_len         = sizeof(MYSNMPAGENT_MIB2_SYSCONTACT) - 1;
static u8_t mysnmpagent_sysname[256]            = MYSNMPAGENT_MIB2_SYSNAME;
static u16_t mysnmpagent_sysname_len            = sizeof(MYSNMPAGENT_MIB2_SYSNAME) - 1;
static u8_t mysnmpagent_syslocation[256]        = MYSNMPAGENT_MIB2_SYSLOCATION;
static u16_t mysnmpagent_syslocation_len        = sizeof(MYSNMPAGENT_MIB2_SYSLOCATION) - 1;

/* Heap statistics */
#if MBED_HEAP_STATS_ENABLED
mbed_stats_heap_t heap_stats;
#endif

/* Sleep interval */
#define SLEEP_INTERVAL  2000ms

#if MBED_CONF_MBED_TRACE_ENABLE

static void trace_mutex_lock();
static void trace_mutex_unlock();

#endif /* #if MBED_CONF_MBED_TRACE_ENABLE */

#if LWIP_SNMP_V3
extern "C" {
void snmpv3_dummy_init(void);
}
#endif

int main()
{
#if MBED_CONF_MBED_TRACE_ENABLE
    /* Necessary for mbed-trace across threads */
    mbed_trace_mutex_wait_function_set( trace_mutex_lock );
    mbed_trace_mutex_release_function_set( trace_mutex_unlock );
    /* Initialize mbed-trace library */
    mbed_trace_init();
    /* Only trace the listed groups */
    mbed_trace_include_filters_set("Main,lwIP");
    /* Trace level and color */
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_INFO | TRACE_MODE_COLOR);
#endif

    /* Mbed OS version */
    tr_info("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    /* Connect to the network */
    tr_info("Connecting to the network...");
    auto net = NetworkInterface::get_default_instance();
    if (net == NULL) {
        tr_err("No Network interface found.");
        return EXIT_FAILURE;
    }
    auto rc = net->connect();
    if (rc != 0) {
        tr_err("Connection error: %d", rc);
        return EXIT_FAILURE;
    }

    /* MAC address */
    const char *macaddr = net->get_mac_address();
    tr_info("MAC: %s", macaddr ? macaddr : "N/A");

    /* IP address */
    SocketAddress sockaddr;
    rc = net->get_ip_address(&sockaddr);
    if (rc != NSAPI_ERROR_OK) {
        tr_err("Get local IP address failed: %d", rc);
        return EXIT_FAILURE;
    }
    tr_info("IP: %s", sockaddr.get_ip_address());

    tr_info("Connection Success");

    /* Enable lwIP stack
     *
     * The SNMP Agent code, extracting from lwIP repository, relies on lwIP stack.
     * Due to singleton, this will do (and won't redo) lwIP stack initialization.
     */
    LWIP &lwip = LWIP::get_instance();

    /* Set up SNMP device enterprise OID */
    snmp_set_device_enterprise_oid(&mysnmpagent_device_enterprise_oid);

#ifdef MYSNMPAGENT_TRAP_DST_IP
    /* Set up one SNMP trap destination */
    snmp_trap_dst_enable(0, 1);

    ip_addr_t ipaddr_trap_dst;
    ipaddr_aton(mysnmpagent_trap_dst_ip, &ipaddr_trap_dst);
    snmp_trap_dst_ip_set(0, &ipaddr_trap_dst);
#endif

    /* Enable authentication traps */
    snmp_set_auth_traps_enabled(SNMP_AUTH_TRAPS_ENABLED);

    /* Set up SNMP MIB-2 system group 1.3.6.1.2.1.1 */
    snmp_mib2_set_sysdescr(mysnmpagent_sysdescr, &mysnmpagent_sysdescr_len);
    snmp_mib2_set_syscontact(mysnmpagent_syscontact, &mysnmpagent_syscontact_len, sizeof(mysnmpagent_syscontact) - 1);
    snmp_mib2_set_sysname(mysnmpagent_sysname, &mysnmpagent_sysname_len, sizeof(mysnmpagent_sysname) - 1);
    snmp_mib2_set_syslocation(mysnmpagent_syslocation, &mysnmpagent_syslocation_len, sizeof(mysnmpagent_syslocation) - 1);

    /* Set up SNMP community string */
    snmp_set_community(MYSNMPAGENT_COMMUNITY);

    /* Set up SNMP community string for write-access */
    snmp_set_community_write(MYSNMPAGENT_COMMUNITY_WRITE);

    /* Set up SNMP community string for sending traps */
    snmp_set_community_trap(MYSNMPAGENT_COMMUNITY_TRAP);

    /* Set up synchronization for MIB-2 which needs to access lwIP stats from lwIP thread */
    snmp_threadsync_init(&snmp_mib2_lwip_locks, snmp_mib2_lwip_synchronizer);

#if LWIP_SNMP_V3
    snmpv3_dummy_init();
#endif

    /* Set up SNMP MIBs */
    snmp_set_mibs(mysnmpagent_mibs, LWIP_ARRAYSIZE(mysnmpagent_mibs));

    /* SNMP community names */
    tr_info("SNMP community: %s", snmp_get_community());
    tr_info("SNMP community for write-access: %s", snmp_get_community_write());
    tr_info("SNMP community for sending traps: %s", snmp_get_community_trap());

    /* Enable SNMP Agent running in separate thread
     *
     * By default (SNMP_USE_NETCONN=1, SNMP_USE_RAW=0), SNMP Agent runs in a
     * worker thread instead of lwIP tcp/ip thread, so that blocking operations
     * can be done in MIB calls.
     *
     * Continuing above, netconn is replaced with Mbed transport.
     */
    snmp_init();

    /* Idle in the main routine */
    while (true) {
        ThisThread::sleep_for(SLEEP_INTERVAL);
    }

    return EXIT_SUCCESS;
}

#if MBED_CONF_MBED_TRACE_ENABLE

/* Synchronize all mbed-trace log output with mutex */
static Mutex trace_mutex;

static void trace_mutex_lock()
{
    trace_mutex.lock();
}

static void trace_mutex_unlock()
{
    trace_mutex.unlock();
}

#endif  /* #if MBED_CONF_MBED_TRACE_ENABLE */
