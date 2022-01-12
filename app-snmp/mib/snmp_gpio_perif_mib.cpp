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

#if LWIP_SNMP

/* SNMP includes */
#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_scalar.h"
#include "snmp_agent_config.h"

/* Mbed includes */
#include "mbed.h"

/* --- gpio peripheral MIB .1.3.6.1.4.1.<vendor>.1 --- */

/* Declare access functions */
static s16_t buttons_get_value(const struct snmp_scalar_array_node_def *node, void *value);
static s16_t leds_get_value(const struct snmp_scalar_array_node_def *node, void *value);
static snmp_err_t leds_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);
static snmp_err_t leds_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value);

/* buttons .1.3.6.1.4.1.<vendor>.1.1 */
static const struct snmp_scalar_array_node_def button_nodes[] = {
    {1, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_ONLY},  // BUTTON1
    {2, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_ONLY},  // BUTTON2
};

static const struct snmp_scalar_array_node buttons_node =
    SNMP_SCALAR_CREATE_ARRAY_NODE(1,
                                  button_nodes,
                                  buttons_get_value,
                                  NULL,
                                  NULL);

/* leds .1.3.6.1.4.1.<vendor>.1.2 */
static const struct snmp_scalar_array_node_def led_nodes[] = {
    {1, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE}, // LED1
    {2, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE}, // LED2
};

static const struct snmp_scalar_array_node leds_node =
    SNMP_SCALAR_CREATE_ARRAY_NODE(2,
                                  led_nodes,
                                  leds_get_value,
                                  leds_set_test,
                                  leds_set_value);

static const struct snmp_node* const gpio_perif_mib_nodes[] = {
    &buttons_node.node.node,
    &leds_node.node.node
};

/* --- gpio peripheral MIB .1.3.6.1.4.1.<vendor>.1 --- */
static const struct snmp_tree_node gpio_perif_mib_root = SNMP_CREATE_TREE_NODE(1, gpio_perif_mib_nodes);

static const u32_t gpio_perif_mib_base_oid_arr[] = {1, 3, 6, 1, 4, 1, MYSNMPAGENT_VENDOR_ENTERPRISE_OID, 1};

extern "C"
const struct snmp_mib gpio_perif_mib = SNMP_MIB_CREATE(gpio_perif_mib_base_oid_arr, &gpio_perif_mib_root.node);

/*----------------------------------------------------------------------------*/

static InterruptIn button1(MBED_CONF_APP_GPIO_PERIF_BUTTON1);
static InterruptIn button2(MBED_CONF_APP_GPIO_PERIF_BUTTON2);
static DigitalOut led1(MBED_CONF_APP_GPIO_PERIF_LED1);
static DigitalOut led2(MBED_CONF_APP_GPIO_PERIF_LED2);

/* buttons instance .1.3.6.1.4.1.<vendor>.1.1.<n>.0 */

static s16_t buttons_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
    int button_value;

    switch (node->oid) {
        case 1: /* BUTTON1 */
            button_value = (int) button1;
            break;

        case 2: /* BUTTON2 */
            button_value = (int) button2;
            break;

        default:
            LWIP_DEBUGF(SNMP_MIB_DEBUG, ("buttons_get_value(): unknown id: %"S32_F"\n", node->oid));
            return 0;
    }

    s32_t *int_ptr = (s32_t *) value;
    *int_ptr = button_value;

    return sizeof(*int_ptr);
}

/* leds instance .1.3.6.1.4.1.<vendor>.1.2.<n>.0 */

static s16_t leds_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
    int led_value;

    switch (node->oid) {
        case 1: /* LED1 */
            led_value = (int) led1;
            break;

        case 2: /* LED2 */
            led_value = (int) led2;
            break;

        default:
            LWIP_DEBUGF(SNMP_MIB_DEBUG, ("leds_get_value(): unknown id: %"S32_F"\n", node->oid));
            return 0;
    }

    s32_t *int_ptr = (s32_t *) value;
    *int_ptr = led_value;

    return sizeof(*int_ptr);
}

static snmp_err_t leds_set_test(const struct snmp_scalar_array_node_def *node, u16_t len, void *value)
{
    LWIP_UNUSED_ARG(len);
    LWIP_UNUSED_ARG(value);

    switch (node->oid) {
        case 1: /* LED1 */
        case 2: /* LED2 */
            break;

        default:
            LWIP_DEBUGF(SNMP_MIB_DEBUG, ("leds_set_test(): unknown id: %"S32_F"\n", node->oid));
            return SNMP_ERR_GENERROR;
    }

    return SNMP_ERR_NOERROR;
}

static snmp_err_t leds_set_value(const struct snmp_scalar_array_node_def *node, u16_t len, void *value)
{
    int led_value = *((u32_t*) value) ? 1 : 0;

     switch (node->oid) {
        case 1: /* LED1 */
            led1 = led_value;
            break;

        case 2: /* LED2 */
            led2 = led_value;
            break;

        default:
            LWIP_DEBUGF(SNMP_MIB_DEBUG, ("leds_set_value(): unknown id: %"S32_F"\n", node->oid));
            return SNMP_ERR_GENERROR;
    }

    return SNMP_ERR_NOERROR;
}

#endif /* LWIP_SNMP */
