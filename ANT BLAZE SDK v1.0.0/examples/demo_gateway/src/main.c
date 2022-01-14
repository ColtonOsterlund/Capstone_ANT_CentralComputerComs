/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2016
All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nrf.h"
#include "nrf_sdm.h"
#include "bsp.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"
#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "app_error.h"
#include "app_timer.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_blaze_gateway_interface.h"
#include "ant_blaze_defines.h"
#include "events.h"
#include "ant_stack_handler_types.h"
#include "ant_interface.h"
#include "sdk_config.h"

#define TIMER_TICKS                         APP_TIMER_TICKS(ANT_BLAZE_TIMEOUT_INTERVAL)
APP_TIMER_DEF(m_timer_id);

#define DEMO_TX_POWER                       RADIO_TX_POWER_LVL_3
#define DEMO_CHANNEL_PERIOD                 ANT_BLAZE_CHANNEL_PERIOD_DYNAMIC    /**< Set to valid ANT channel period or ANT_BLAZE_CHANNEL_PERIOD_DYNAMIC */

#define DEMO_FREQ_NUM                       ((uint8_t) 1)     /**< Set to number of desired radio frequencies (1 - 3). */
#define DEMO_FREQ_A                         ((uint8_t) 11)    /**< 2411MHz. */
#define DEMO_FREQ_B                         ((uint8_t) 22)    /**< 2422MHz. */
#define DEMO_FREQ_C                         ((uint8_t) 33)    /**< 2433MHz. */

#define DEMO_USE_ENCRYPTION                 ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED

#define DEMO_NETWORK_ID                     ((uint16_t) 20000)      /**< Network ID*/
#define DEMO_NODE_ID                        ((uint16_t) 300)        /**< 9-bit node address*/

#define VERSION_STRING                      "BJP1.00B00"

// Application specific messaging
#define PING_REQUEST                        0x01

#define PING_REQUESTED_PACKETS_MASK         0x0F
#define PING_REQUESTED_PAYLOAD_MASK         0x10
#define PING_PARALLEL_PING_RANGE_MASK       0x7F

typedef struct
{
    uint16_t lower_id; /**< Lower node id of parallel ping request. */
    uint16_t upper_id; /**< Upper node id of parallel ping request. Set equal to lower_id to ping a single node */
    uint8_t number_packets_requested; /**< The number of packets that should be sent back in the ping response. Range: 1-8 inclusive. */
    uint8_t length_of_additional_payload_bytes; /**< The number of additional payload bytes being included. Must be less than or equal to 35*/
    uint8_t* p_additional_payload; /**< Pointer to the additional payload bytes. */
} ping_request_t;

volatile uint32_t g_event_flags   = 0;

static uint8_t m_tx_payload[ANT_BLAZE_MAX_MESSAGE_LENGTH];

static uint8_t m_ant_network_key[] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; // ANT Public network key. Replace with manufacturer specific ANT network key assigned by Dynastream.
static uint8_t m_encryption_key[] = {0x7D, 0x77, 0xBE, 0xE8, 0xD2, 0xE3, 0x2B, 0x2F, 0x41, 0x4C, 0x7C, 0x30, 0x89, 0xC1, 0x59, 0x1D};  // Encryption key to use for this network

static ant_blaze_gateway_config_t m_ant_blaze_gateway_config; // Configuration parameters for the gateway.
static uint8_t timer_counter = 0;


/**@brief Process one second timer event
 * Runs in IRQ context, and generates an event so timeout can be processed in app context.
 */
static void one_sec_timer_event(void * p_context)
{
    EVENT_SET(EVENT_TIMEOUT);
}


/**@brief Function for handling SoftDevice asserts.
 *
 */
void softdevice_assert_callback(uint32_t id, uint32_t pc, uint32_t info)
{
    app_error_fault_handler(id, pc, info);
}

/**@brief Function which sends a set of parallel ping requests
 *
 * This function can be used to send a ping request message into the network.
 *
 * @param[in] ping_request The details of the ping request.
 */
static void send_ping_request(ping_request_t ping_request)
{
    ant_blaze_message_t tx_message;

    memset(m_tx_payload, 0, ANT_BLAZE_MAX_MESSAGE_LENGTH); // Clear the tx buffer
    tx_message.p_data = m_tx_payload;

    tx_message.length = ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH;
    m_tx_payload[0] = PING_REQUEST;
    m_tx_payload[3] = ping_request.number_packets_requested; // Number of packets to send in response (max 8)

    // If sending additional bytes which are to be copied into bytes 5+ of the response
    if( (ping_request.length_of_additional_payload_bytes != 0) &&
        (ping_request.number_packets_requested > 1) &&
        (ping_request.p_additional_payload != NULL) )
    {
        m_tx_payload[3] |=  (1 << 4); // Set bit 4 to 1 to indicate bytes 5+ of the payload are to be copied into bytes 5+ of the response
        memcpy(&m_tx_payload[5], ping_request.p_additional_payload, ping_request.length_of_additional_payload_bytes);
        tx_message.length = ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH + ping_request.length_of_additional_payload_bytes;
    }

    if(ping_request.lower_id == ping_request.upper_id)
    {
        tx_message.address = ping_request.upper_id; // Send this ping to a specific node ID (either an individual node or group address)
        m_tx_payload[1] = 0;
        m_tx_payload[2] = 0;
    }
    else
    {
        tx_message.address = 0; // Send parallel ping to all nodes
        m_tx_payload[1] = ping_request.lower_id & 0xFF; // Parallel Ping - Start of Range (Address LSB)
        m_tx_payload[2] = (ping_request.lower_id >> 1) & 0x80; // Bit 7: Parallel Ping - Start of Range (Address Bit 8)
        m_tx_payload[2] |= ((ping_request.upper_id - ping_request.lower_id) & PING_PARALLEL_PING_RANGE_MASK); // Bits 0-6: Range of Parallel Ping
    }

    // Send the message into the network
    ant_blaze_gateway_send_message(&tx_message);

    NRF_LOG_INFO("Tx mesh message - Address: %d\r\n", tx_message.address);
    NRF_LOG_HEXDUMP_INFO(tx_message.p_data, tx_message.length);
}


/**@brief Function for handling BSP events
 * Runs in app context.
 */
static void bsp_evt_handler(bsp_event_t evt)
{
    ping_request_t ping_req = { .number_packets_requested = 5,
                                .length_of_additional_payload_bytes = 0,
                                .p_additional_payload = NULL
                              };
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            NRF_LOG_INFO("Ping 1 - 2\n");
            ping_req.lower_id = 1;
            ping_req.upper_id = 2;
            send_ping_request(ping_req);
            break;

        case BSP_EVENT_KEY_1:
            NRF_LOG_INFO("Ping 3 - 4\n");
            ping_req.lower_id = 3;
            ping_req.upper_id = 4;
            send_ping_request(ping_req);
            break;

        case BSP_EVENT_KEY_2:
            NRF_LOG_INFO("Ping everyone!\n");
            ping_req.lower_id = 0;
            ping_req.upper_id = 0;
            send_ping_request(ping_req);
            break;

        default:
            return; // no implementation needed
    }
}


/**@brief Function for the Timer and BSP initialization.
 * Runs in app context.
 */
static void utils_init(void)
{
    uint32_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                        bsp_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                one_sec_timer_event);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received. Must be executed in IRQ context.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
}


/**@brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event interrupt.
 * Runs in app context.
 */
static void softdevice_init(void)
{
    // Enable SoftDevice.
    uint32_t err_code;
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    err_code = sd_softdevice_enable(&clock_lf_cfg, softdevice_assert_callback, ANT_LICENSE_KEY);
    APP_ERROR_CHECK(err_code);

    // Set application IRQ to lowest priority.
    err_code = sd_nvic_SetPriority(SD_EVT_IRQn, APP_IRQ_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    // Enable application IRQ (triggered from protocol).
    err_code = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config(); // If additional channels need to be enable, edit ant_stack_config_defs.h
    APP_ERROR_CHECK(err_code);

}



/**@brief Function for handling messages received over
 * the ANT BLAZE network.
 * Runs in app context.
 * @param rx_msg            Message received
 */
static void rx_msg_handler(ant_blaze_message_t rx_msg)
{
    NRF_LOG_INFO("Rx mesh message - Index: %d   Address: %d\r\n", rx_msg.index, rx_msg.address);
    NRF_LOG_HEXDUMP_INFO(rx_msg.p_data, rx_msg.length);
}


/**@brief Function to get and handle ANT events.
 * Runs in app context.
 */
void poll_for_ant_events(void)
{
    static uint32_t err_code = NRF_SUCCESS;
    ant_evt_t ant_event;

    // Extract and process all pending ANT events as long as there are any left.
    do
    {
        err_code = sd_ant_event_get(&(ant_event.channel), &(ant_event.event), ant_event.msg.evt_buffer);
        if (err_code == NRF_SUCCESS)
        {
            ant_blaze_gateway_process_channel_event(&ant_event);
        }
    } while (err_code == NRF_SUCCESS);
}

/**@brief Function to get and handle SOC events.
 * Must be executed in IRQ context.
 */
void poll_for_soc_events(void)
{
    static uint32_t err_code = NRF_SUCCESS;
    uint32_t evt_id;

    // Extract and process all pending SOC events as long as there are any left.
    do
    {
         // Pull event from SOC.
        err_code = sd_evt_get(&evt_id);
        if (err_code == NRF_SUCCESS)
        {
            sys_evt_dispatch(evt_id);
        }
    } while (err_code == NRF_SUCCESS);
}

/**@brief Function to get and handle application events.
 * Runs in app context.
 */
void poll_for_events(void)
{
    uint32_t local_flags = g_event_flags;
    uint16_t nodes_in_range = 0;
    uint32_t err_code;

    do
    {
        if(local_flags & EVENT_ANT_STACK)
        {
            EVENT_CLEAR(EVENT_ANT_STACK);
            poll_for_ant_events();
            poll_for_soc_events(); // Handle SOC events in IRQ context
        }
        if(local_flags & EVENT_TIMEOUT)
        {
            EVENT_CLEAR(EVENT_TIMEOUT);
            ant_blaze_gateway_process_timeout();
            // Log current node density every ~30 s
            timer_counter++;
            if(timer_counter % 30 == 0)
            {
                err_code = ant_blaze_gateway_get_number_of_nodes_in_range(&nodes_in_range);
                if(err_code == NRF_SUCCESS)
                {
                    NRF_LOG_INFO("Current nodes in range: %d\r\n", nodes_in_range);
                }
            }
        }
        local_flags = g_event_flags;
    } while (local_flags != 0);
}


/**@brief Function for stack interrupt handling.
 */
void SD_EVT_IRQHandler(void)
{
    EVENT_SET(EVENT_ANT_STACK); // Set an event when receiving an interrupt from the stack so message processing
                                // takes place in app context.
}

/** @brief The main function
 */
int main(void)
{
    uint32_t err_code;

    utils_init();
    softdevice_init();

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize the ANT BLAZE Gateway library
    err_code = ant_blaze_gateway_init(rx_msg_handler, NULL, app_error_handler, ANT_BLAZE_EVALUATION_LICENSE_KEY);
    APP_ERROR_CHECK(err_code);

    // Configure the gateway library.
    // This step must be performed before starting the gateway library.
    memset(&m_ant_blaze_gateway_config, 0, sizeof(ant_blaze_gateway_config_t)); // clear the contents of m_ant_blaze_gateway_config

    m_ant_blaze_gateway_config.node_id = DEMO_NODE_ID;
    m_ant_blaze_gateway_config.network_id = DEMO_NETWORK_ID;

    m_ant_blaze_gateway_config.num_channels = DEMO_FREQ_NUM;
    m_ant_blaze_gateway_config.radio_freqs[0] = DEMO_FREQ_A;
    m_ant_blaze_gateway_config.radio_freqs[1] = DEMO_FREQ_B;
    m_ant_blaze_gateway_config.radio_freqs[2] = DEMO_FREQ_C;

    m_ant_blaze_gateway_config.tx_power = DEMO_TX_POWER;
    m_ant_blaze_gateway_config.encryption_enabled = DEMO_USE_ENCRYPTION;
    m_ant_blaze_gateway_config.p_ant_network_key = m_ant_network_key;
    m_ant_blaze_gateway_config.p_encryption_key = m_encryption_key;
    err_code = ant_blaze_gateway_config(&m_ant_blaze_gateway_config);
    APP_ERROR_CHECK(err_code);

    // Start the ANT BLAZE library
    err_code = ant_blaze_gateway_start();
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Start ANT BLAZE Library\r\n");

    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);

    // Enter main loop
    for (;;)
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        poll_for_events();

        // Print out buffered logs
        NRF_LOG_FLUSH();
    }
}
