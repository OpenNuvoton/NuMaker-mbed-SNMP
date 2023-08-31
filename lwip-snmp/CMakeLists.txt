# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(lwip-snmp INTERFACE)

target_include_directories(lwip-snmp
    INTERFACE
        .
        lwip/src/apps/snmp
        lwip/src/include/lwip/apps
        ./lwip/src/include
        ./lwip/src/include/lwip
)

target_sources(lwip-snmp
    INTERFACE
        lwip/src/apps/snmp/snmp_asn1.c
        lwip/src/apps/snmp/snmp_core.c
        lwip/src/apps/snmp/snmp_mib2.c
        lwip/src/apps/snmp/snmp_mib2_icmp.c
        lwip/src/apps/snmp/snmp_mib2_interfaces.c
        lwip/src/apps/snmp/snmp_mib2_ip.c
        lwip/src/apps/snmp/snmp_mib2_snmp.c
        lwip/src/apps/snmp/snmp_mib2_system.c
        lwip/src/apps/snmp/snmp_mib2_tcp.c
        lwip/src/apps/snmp/snmp_mib2_udp.c
        lwip/src/apps/snmp/snmp_msg.c
        lwip/src/apps/snmp/snmp_pbuf_stream.c
        lwip/src/apps/snmp/snmp_scalar.c
        lwip/src/apps/snmp/snmp_snmpv2_framework.c
        lwip/src/apps/snmp/snmp_snmpv2_usm.c
        lwip/src/apps/snmp/snmp_table.c
        lwip/src/apps/snmp/snmp_threadsync.c
        lwip/src/apps/snmp/snmp_traps.c
        lwip/src/apps/snmp/snmpv3.c
        lwip/src/apps/snmp/snmpv3_mbedtls.c
	lwip/src/apps/snmp/snmpv3_dummy.c
)

