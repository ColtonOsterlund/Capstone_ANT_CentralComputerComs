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
#include "sdk_config.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"
#include "nrf_log_ctrl.h"
#include "nrf_log.h"
#include "app_error.h"
#include "app_timer.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_blaze_node_interface.h"
#include "ant_blaze_defines.h"
#include "events.h"
#include "ant_stack_handler_types.h"
#include "ant_interface.h"

#if defined(BLE_STACK_SUPPORT_REQD)
#include "ble_app_interface.h"
#include "ble_stack_handler_types.h"
#include "ble_advertising.h"
#include "ble.h"
#endif

#define TIMER_TICKS                         APP_TIMER_TICKS(ANT_BLAZE_TIMEOUT_INTERVAL)
APP_TIMER_DEF(m_timer_id);

#define DEMO_TX_POWER                       RADIO_TX_POWER_LVL_3
#define DEMO_CHANNEL_PERIOD                 ANT_BLAZE_CHANNEL_PERIOD_DYNAMIC
#define DEMO_FREQ_NUM                       ((uint8_t) 1)     /**< Set to number of desired radio frequencies (1 - 3).  */
#define DEMO_FREQ_A                         ((uint8_t) 11)    /**< 2411MHz. */
#define DEMO_FREQ_B                         ((uint8_t) 22)    /**< 2422MHz. */
#define DEMO_FREQ_C                         ((uint8_t) 33)    /**< 2433MHz. */
#define DEMO_USE_ENCRYPTION                 ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED

#define DEMO_NETWORK_ID                     ((uint16_t) 20000)
#define DEMO_NUM_GROUP_ADDRESSES            ((uint16_t) 32)
#define DEMO_GROUP_A                        ((uint16_t) 511)
#define DEMO_GROUP_B                        ((uint16_t) 500)

/**< Select between one of the following options. */
#define NODE_ID_FROM_SWITCHES               // Derive the node ID from the dip switches in the battery board
//#define NODE_ID_FROM_DEVICE_ID              // Derive the node ID from the 9 LSB of the internal Device ID

#define VERSION_STRING                      "BIO1.00B00"

// Application specific messaging
#define PING_REQUEST                        0x01
#define PING_RESPONSE                       0x02

#define PING_RF_FREQ_MASK                   0x7F
#define PING_RF_FREQ_UNKNOWN                0x7F
#define PING_PERIOD_MASK                    0x07
#define PING_REQUESTED_PACKETS_MASK         0x0F
#define PING_REQUESTED_PAYLOAD_MASK         0x10

typedef enum
{
    ONE_HZ = 0,
    TWO_HZ = 1,
    FOUR_HZ = 2,
    EIGHT_HZ = 3,
    SIXTEEN_HZ = 4,
    PERIOD_UNKNOWN = 7
} encoded_channel_period_t;


typedef struct
{
    uint16_t node_id;
    uint16_t network_id;
} nvm_config_t;

volatile uint32_t g_event_flags   = 0;

static uint8_t m_ping_counter = 0;

static uint8_t m_tx_payload[ANT_BLAZE_MAX_MESSAGE_LENGTH];

static uint8_t m_ant_network_key[] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; // ANT Public network key. Replace with manufacturer specific ANT network key assigned by Dynastream.
static uint8_t m_encryption_key[] = {0x7D, 0x77, 0xBE, 0xE8, 0xD2, 0xE3, 0x2B, 0x2F, 0x41, 0x4C, 0x7C, 0x30, 0x89, 0xC1, 0x59, 0x1D};  // Encryption key to use for this network

static ant_blaze_node_config_t m_ant_blaze_node_config;


/**@brief One second timer event
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


/**@brief Function for the Timer and BSP initialization.
 * Runs in app context.
 */
static void utils_init(void)
{
    uint32_t err_code;

    err_code = app_timer_init();
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
    #ifdef BLE_STACK_SUPPORT_REQD
    ble_advertising_on_sys_evt(sys_evt);
    #endif
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

    // Configure ANT stack
    err_code = ant_stack_static_config(); // If additional channels need to be enable, edit sdk_config.h
    APP_ERROR_CHECK(err_code);

    #ifdef BLE_STACK_SUPPORT_REQD
    // Enable BLE Stack
    bai_stack_init();
    #endif
}


/**@brief Initializes the battery board switches as inputs.
 * Runs in app context.
 */
static void switch_init(void)
{
#if defined(SWITCHES_MASK)
    for (uint32_t pin = 0; pin < 32; pin++)
    {
        if((SWITCHES_MASK) & (1 << pin))
        {
            nrf_gpio_cfg_input(pin, SWITCH_PULL);
        }
    }
#endif
}


/**@brief Function for handling messages received over
 * the ANT BLAZE network.
 * Runs in app context.
 * @param rx_msg            Message received
 */
static void rx_msg_handler(ant_blaze_message_t rx_msg)
{
    uint8_t rf_freq = PING_RF_FREQ_UNKNOWN;
    encoded_channel_period_t encoded_period = PERIOD_UNKNOWN;
    uint16_t current_period = 0;
    uint8_t requested_packets;
    uint16_t parallel_range_lower;
    uint16_t parallel_range_upper;
    ant_blaze_message_t tx_message;

    memset(m_tx_payload, 0, ANT_BLAZE_MAX_MESSAGE_LENGTH);
    memset(&tx_message, 0, sizeof(ant_blaze_message_t));
    tx_message.p_data = m_tx_payload;

    NRF_LOG_INFO("Rx mesh message - Index: %d   Address: %d\r\n", rx_msg.index, rx_msg.address);
    NRF_LOG_HEXDUMP_INFO(rx_msg.p_data, rx_msg.length);

    #ifdef BLE_STACK_SUPPORT_REQD
    bai_update_rx_char(&rx_msg);
    #endif

    switch(rx_msg.p_data[0])
    {
        case PING_REQUEST:
            if(ant_blaze_node_get_current_scan_frequency(&rf_freq) != NRF_SUCCESS)
            {
                rf_freq = PING_RF_FREQ_UNKNOWN;
            }

            if(ant_blaze_node_get_current_channel_period(&current_period) == NRF_SUCCESS)
            {
                switch(current_period)
                {
                    case ANT_BLAZE_CHANNEL_PERIOD_1HZ:
                        encoded_period = ONE_HZ;
                        break;
                    case ANT_BLAZE_CHANNEL_PERIOD_2HZ:
                        encoded_period = TWO_HZ;
                        break;
                    case ANT_BLAZE_CHANNEL_PERIOD_4HZ:
                        encoded_period = FOUR_HZ;
                        break;
                    case ANT_BLAZE_CHANNEL_PERIOD_8HZ:
                        encoded_period = EIGHT_HZ;
                        break;
                    case ANT_BLAZE_CHANNEL_PERIOD_16HZ:
                        encoded_period = SIXTEEN_HZ;
                    default:
                        break;
                }
            }

            // Construct ping response
            m_tx_payload[0] = PING_RESPONSE;
            m_tx_payload[1] = 0xFF;
            m_tx_payload[2] = (encoded_period & PING_PERIOD_MASK);
            m_tx_payload[3] = (rf_freq & PING_RF_FREQ_MASK);
            m_tx_payload[4] = m_ping_counter;
            m_ping_counter++;

            // Check whether there is a lower and upper range of addresses in the request
            if((rx_msg.p_data[1] != 0) || (rx_msg.p_data[2] != 0))
            {
                parallel_range_lower = 0x00FF & rx_msg.p_data[1];
                parallel_range_lower |=  ((uint16_t)(0x80 & rx_msg.p_data[2]) << 1);
                parallel_range_upper = parallel_range_lower + (0x7F & rx_msg.p_data[2]);

                // Ignore request if node is not within specified range
                if( (m_ant_blaze_node_config.node_id < parallel_range_lower) ||
                    (m_ant_blaze_node_config.node_id > parallel_range_upper))
                {
                    return;
                }
            }

            // Send multipacket response with requested number of packets
            // Payload for all other packets except by the first one will be zeroes
            requested_packets = rx_msg.p_data[3] & PING_REQUESTED_PACKETS_MASK;
            // Ignore requests that exceed maximum valid packet count
            if(requested_packets > ANT_BLAZE_MAX_MESSAGE_LENGTH/ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH)
            {
                return;
            }
            tx_message.length = requested_packets*ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH;
            if(rx_msg.p_data[3] & PING_REQUESTED_PAYLOAD_MASK)
            {
                // Use payload provided by the gateway in the response
                // Requested length must match gateway message length to ensure data can be copied
                if((tx_message.length > ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH) &&
                   (tx_message.length <= rx_msg.length))
                {
                    memcpy(&m_tx_payload[ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH], &rx_msg.p_data[ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH], tx_message.length - ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH);
                }
            }
            ant_blaze_node_send_message(&tx_message);
            break;

        default:
            break;
    }
}

#ifdef BLE_STACK_SUPPORT_REQD
static void ble_ant_sf_event_handler(uint8_t* p_message, uint32_t length)
{
    // Application can handle here messages received over BLE
}
#endif


/**@brief Function for handling messages received on the backchannel
 * of the ANT BLAZE master channel(s)
 * Runs in app context.
 * @param   p_ant_evt       Pointer to ANT event
 */
void backchannel_msg_handler(ant_evt_t* p_ant_evt)
{
    // Application can handle here messages received over the backchannel
    // of the ANT BLAZE master channel(s)
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
            ant_blaze_node_process_channel_event(&ant_event);   // Must be called from app context
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

#ifdef BLE_STACK_SUPPORT_REQD
void poll_for_ble_events(void)
{

    uint32_t err_code;
    uint32_t ble_evt_buffer[CEIL_DIV(BLE_STACK_EVT_MSG_BUF_SIZE, sizeof(uint32_t))];  /**< Buffer for receiving BLE events from the SoftDevice. */
    uint16_t evt_len = sizeof(ble_evt_buffer);

    do
    {
        // Pull event from stack
        err_code = sd_ble_evt_get((uint8_t*)&ble_evt_buffer, &evt_len);
        if(err_code == NRF_SUCCESS)
        {
            bai_evt_dispatch((ble_evt_t*)&ble_evt_buffer);
        }
    } while (err_code == NRF_SUCCESS);
}
#endif

/**@brief Function to get and handle ANT, BLE and
 * timer events.
 * This function runs in app context.
 */
void poll_for_events(void)
{
    uint32_t local_flags = g_event_flags;

    do
    {
        if(local_flags & EVENT_ANT_STACK)
        {
            EVENT_CLEAR(EVENT_ANT_STACK);
            poll_for_ant_events();
            #ifdef BLE_STACK_SUPPORT_REQD
            poll_for_ble_events();
            #endif
            poll_for_soc_events(); // Handle SOC events
        }
        if(local_flags & EVENT_TIMEOUT)
        {
            EVENT_CLEAR(EVENT_TIMEOUT);
            ant_blaze_node_process_timeout();   // Must be called from app context
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

/**@brief Get node and network ID to configure in this device
 * @param p_node_id     Pointer to variable where node ID will be stored
 * @param p_network_id  Pointer to variable where network ID will be stored
 */
void get_node_and_network_id(uint16_t* p_node_id, uint16_t* p_network_id)
{
    *p_network_id = DEMO_NETWORK_ID;

    #if defined(NODE_ID_FROM_SWITCHES)
    // Set the node ID by reading value in dip switches. Switch 1 is the lsb, switch 8 the msb.
    *p_node_id = ((nrf_gpio_pin_read(SWITCH_1) << 0) |
                  (nrf_gpio_pin_read(SWITCH_2) << 1) |
                  (nrf_gpio_pin_read(SWITCH_3) << 2) |
                  (nrf_gpio_pin_read(SWITCH_4) << 3) |
                  (nrf_gpio_pin_read(SWITCH_5) << 4) |
                  (nrf_gpio_pin_read(SWITCH_6) << 5) |
                  (nrf_gpio_pin_read(SWITCH_7) << 6) |
                  (nrf_gpio_pin_read(SWITCH_8) << 7));
    #elif defined(NODE_ID_FROM_DEVICE_ID)
    *p_node_id = ((uint16_t)(NRF_FICR->DEVICEID[0]&0x01FF)); // Set 9-bit node address based on internal Device ID.
    #endif
}


/** @brief The main function
 */
int main(void)
{
    uint32_t err_code;

    switch_init();
    utils_init();
    softdevice_init();

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    // Use DC-DC Converter (recommended to optimize power consumption on D52 module)
    err_code = sd_power_dcdc_mode_set(DC_TO_DC_ON);
    APP_ERROR_CHECK(err_code);

    // Initialize the ANT BLAZE library with Evaluation License Key.
    err_code = ant_blaze_node_init(rx_msg_handler, backchannel_msg_handler, app_error_handler, ANT_BLAZE_EVALUATION_LICENSE_KEY);
    APP_ERROR_CHECK(err_code);

    // Configure the ANT BLAZE library.
    // This step must be performed before starting the library.
    memset(&m_ant_blaze_node_config, 0, sizeof(ant_blaze_node_config_t));
    get_node_and_network_id(&(m_ant_blaze_node_config.node_id), &(m_ant_blaze_node_config.network_id));
    m_ant_blaze_node_config.channel_period = DEMO_CHANNEL_PERIOD;
    m_ant_blaze_node_config.radio_freqs[0] = DEMO_FREQ_A;
    m_ant_blaze_node_config.radio_freqs[1] = DEMO_FREQ_B;
    m_ant_blaze_node_config.radio_freqs[2] = DEMO_FREQ_C;
    m_ant_blaze_node_config.num_channels = DEMO_FREQ_NUM;
    m_ant_blaze_node_config.tx_power = DEMO_TX_POWER;
    m_ant_blaze_node_config.num_group_addresses = DEMO_NUM_GROUP_ADDRESSES;
    m_ant_blaze_node_config.encryption_enabled = DEMO_USE_ENCRYPTION;
    m_ant_blaze_node_config.p_ant_network_key = m_ant_network_key;
    m_ant_blaze_node_config.p_encryption_key = m_encryption_key;
    err_code = ant_blaze_node_config(&m_ant_blaze_node_config);
    APP_ERROR_CHECK(err_code);

    // Add node to groups, if using the grouping feature
    err_code = ant_blaze_node_add_to_group(DEMO_GROUP_A);
    APP_ERROR_CHECK(err_code);

    err_code = ant_blaze_node_add_to_group(DEMO_GROUP_B);
    APP_ERROR_CHECK(err_code);

    #ifdef BLE_STACK_SUPPORT_REQD
    // Initialize BLE
    bai_init(ble_ant_sf_event_handler);
    #endif

    // Start timer
    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);

    // Start the ANT BLAZE library
    err_code = ant_blaze_node_start();
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Start ANT BLAZE Library, Node ID %d\r\n", m_ant_blaze_node_config.node_id);

    #ifdef BLE_STACK_SUPPORT_REQD
    // Start BLE advertising
    bai_advertising_start(m_ant_blaze_node_config.node_id);
    #endif

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
