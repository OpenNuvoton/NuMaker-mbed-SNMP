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

#ifndef __SNMP_AGENT_CONFIG_H__
#define __SNMP_AGENT_CONFIG_H__

#include "lwip/apps/snmp.h"

/** SNMP vendor enterprise OID
 *
 * @see https://github.com/ARMmbed/mbed-os/blob/2eb06e76208588afc6cb7580a8dd64c5429a10ce/connectivity/lwipstack/lwip/src/include/lwip/apps/snmp_opts.h#L177-L200
 * @see https://github.com/ARMmbed/mbed-os/blob/2eb06e76208588afc6cb7580a8dd64c5429a10ce/connectivity/lwipstack/lwip/src/apps/snmp/lwip_snmp_core.c#L245-L258
 *
 * @note Default to enterprise OID for lwIP and not suitable for production
 */
#define MYSNMPAGENT_VENDOR_ENTERPRISE_OID   SNMP_LWIP_ENTERPRISE_OID

/** SNMP trap destination IP
 *
 * @note If left unchanged, the default is reserved IP for documentation
 *       (see link below). Here, we use it specially: As a key to change
 *       to the SNMP request source. This is for development when the SNMP
 *       request source and the SNMP trap destination are the same.
 *
 *       Reserved IP addresses:
 *       https://en.wikipedia.org/wiki/Reserved_IP_addresses
 *
 * @note Comment out to disable sending trap message
 */
#define MYSNMPAGENT_TRAP_DST_IP             "192.0.2.0"

/* SNMP MIB-2 system group 1.3.6.1.2.1.1 */

/** mib-2.system.sysDescr */
#define MYSNMPAGENT_MIB2_SYSDESCR           "SNMP Agent on Nuvoton Mbed platform"

/** mib-2.system.sysContact */
#define MYSNMPAGENT_MIB2_SYSCONTACT         "foo@example.com"

/** mib-2.system.sysName */
#define MYSNMPAGENT_MIB2_SYSNAME            ""

/** mib-2.system.sysLocation */
#define MYSNMPAGENT_MIB2_SYSLOCATION        ""

/** SNMP community string */
#define MYSNMPAGENT_COMMUNITY               "public"

/** SNMP community string for write access */
#define MYSNMPAGENT_COMMUNITY_WRITE         "private"

/** SNMP community string for sending traps */
#define MYSNMPAGENT_COMMUNITY_TRAP          "public"

#endif /* ifndef DEMO_CONFIG_H */
