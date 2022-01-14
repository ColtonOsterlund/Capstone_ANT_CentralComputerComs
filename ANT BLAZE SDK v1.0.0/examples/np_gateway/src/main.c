/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2017
All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nordic_common.h"
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
#include "ant_blaze_gateway_interface.h"
#include "ant_blaze_defines.h"
#include "ant_stack_handler_types.h"
#include "ant_interface.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "softdevice_handler_appsh.h"
#include "gateway_serial.h"
#include "app_uart.h"

#define SCHED_MAX_EVENT_DATA_SIZE           MAX(ANT_STACK_EVT_MSG_BUF_SIZE, APP_TIMER_SCHED_EVENT_DATA_SIZE)
#define SCHED_QUEUE_SIZE                    20

#define TIMER_TICKS                         APP_TIMER_TICKS(ANT_BLAZE_TIMEOUT_INTERVAL)
APP_TIMER_DEF(m_timer_id);

#define UART_TX_BUF_SIZE                    256     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                    256     /**< UART RX buffer size. */

#define MESG_SIZE_READ                      ((uint8_t)0x55) // async control flag

static SERIAL_MESSAGE m_rx_message;
static SERIAL_MESSAGE m_tx_message;


/**@brief Process one second timer event
 * Runs in IRQ context, and generates an event so timeout can be processed in app context.
 */
static void timer_event(void * p_context)
{
    ant_blaze_gateway_process_timeout();
}


/**@brief Function for the Timer initialization.
 */
static void utils_init(void)
{
    uint32_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                timer_event);
    APP_ERROR_CHECK(err_code);
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          an ANT message.
 */
static void uart_event_handle(void * p_event_data, uint16_t event_size)
{
    static uint8_t rx_ptr;
    uint8_t rx_byte;
    app_uart_evt_t * p_evt = (app_uart_evt_t*) p_event_data;

    switch (p_evt->evt_type)
    {
        case APP_UART_DATA_READY:
            app_uart_get(&rx_byte);
            NRF_LOG_DEBUG("UART Got byte %#04x\r\n", rx_byte);

            if(!m_rx_message.SERIAL_MESSAGE_ucSize) // we are loking for the sync byte of a message
            {
                if(rx_byte == MESG_TX_SYNC) // This is a valid sync byte
                {
                    m_rx_message.SERIAL_MESSAGE_ucCheckSum = MESG_TX_SYNC;  // init the checksum
                    m_rx_message.SERIAL_MESSAGE_ucSize = MESG_SIZE_READ;   // set the byte pointer to get the size byte
                }
            }
            else if(m_rx_message.SERIAL_MESSAGE_ucSize == MESG_SIZE_READ) // If we are processing the size byte of a message
            {
                m_rx_message.SERIAL_MESSAGE_ucSize = 0; // if the size is invalid we want to reset the rx message
                if(rx_byte <= SERIAL_MESG_MAX_SIZE_VALUE) // make sure this is a valid message
                {
                    m_rx_message.SERIAL_MESSAGE_ucSize = rx_byte;  // save the size of the message
                    m_rx_message.SERIAL_MESSAGE_ucCheckSum ^= rx_byte; // calculate the checksum
                    rx_ptr = 0; // set the byte pointer to start collecting the message.
                }
            }
            else
            {
                m_rx_message.SERIAL_MESSAGE_ucCheckSum ^= rx_byte; // calculate checksum
                if(rx_ptr > m_rx_message.SERIAL_MESSAGE_ucSize) // We have receive the whole message (+ 1 for the message ID)
                {
                    if(!m_rx_message.SERIAL_MESSAGE_ucCheckSum) // the checksum passed
                    {
                        NRF_LOG_DEBUG("UART Rx Correct Checksum\r\n");
                        gateway_serial_message_process(&m_rx_message, &m_tx_message);
                        m_rx_message.SERIAL_MESSAGE_ucSize = 0;  // reset the Rx message
                    }
                    else
                    {
                        NRF_LOG_DEBUG("UART Rx Bad Checksum\r\n");
                        m_rx_message.SERIAL_MESSAGE_ucSize = 0;  // reset the Rx message
                    }
                }
                else // this is a data byte
                {
                    m_rx_message.SERIAL_MESSAGE_aucFramedData[rx_ptr++] = rx_byte; // save the byte
                }
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            m_rx_message.SERIAL_MESSAGE_ucSize = 0;
            NRF_LOG_ERROR("UART Communication Error %d\r\n", p_evt->data.error_communication);
            APP_ERROR_HANDLER(p_evt->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            m_rx_message.SERIAL_MESSAGE_ucSize = 0;
            NRF_LOG_ERROR("UART FIFO Error %d\r\n", p_evt->data.error_code);
            APP_ERROR_HANDLER(p_evt->data.error_code);
            break;

        default:
            break;
    }
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          an ANT message.
 */
static void uart_event_schedule(app_uart_evt_t* p_evt)
{
    app_sched_event_put(p_evt, sizeof(app_uart_evt_t), uart_event_handle);
}


/**@brief  Function for initializing the UART module.
 */
static void uart_init(void)
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_ENABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud57600
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_schedule,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
}

/**@brief Function for processing ANT channel events.
 * @param[in] p_ant_evt Pointer to ANT event
 */
static void ant_event_handler(ant_evt_t* p_ant_evt)
{
    ant_blaze_gateway_process_channel_event(p_ant_evt);
}

/**@brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event handler
 */
static void softdevice_init(void)
{
    uint32_t err_code;
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize SoftDevice.
    err_code = softdevice_handler_init(&clock_lf_cfg, NULL, 0, softdevice_evt_schedule);
    APP_ERROR_CHECK(err_code);

    // Subscribe for ANT events.
   err_code = softdevice_ant_evt_handler_set(ant_event_handler);
   APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for SoC events
   err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
   APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for sending serial messages over UART
 * Runs from the main loop
 * @param p_msg             Pointer to SERIAL_MESSAGE struct to send over UART
 */
static void send_serial_message(SERIAL_MESSAGE* p_msg)
{
    if(p_msg->SERIAL_MESSAGE_ucSize == 0)
    {
        // Nothing to send
        return;
    }

    // Calculate checksum
    p_msg->SERIAL_MESSAGE_ucCheckSum = MESG_TX_SYNC;
    for (int i=0; i<=(p_msg->SERIAL_MESSAGE_ucSize + 1); i++) // have to go two more than size so we include the size and the ID
    {
         p_msg->SERIAL_MESSAGE_ucCheckSum ^= p_msg->aucMessage[i]; // calculate the checksum
    }
    p_msg->SERIAL_MESSAGE_aucMesgData[p_msg->SERIAL_MESSAGE_ucSize] = p_msg->SERIAL_MESSAGE_ucCheckSum;  // Move the calculated checksum to correct location in message

    // Write the sync byte
    while(app_uart_put(MESG_TX_SYNC) != NRF_SUCCESS);
    NRF_LOG_DEBUG("Sent byte %#04x\r\n", MESG_TX_SYNC);

    // Write the rest of the message. Need to go three more than size so we include the size, ID and checksum
    for(int i=0; i<=(p_msg->SERIAL_MESSAGE_ucSize + 2); i++)
    {
        while (app_uart_put(p_msg->aucMessage[i]) != NRF_SUCCESS);
        NRF_LOG_DEBUG("Sent byte %#04x\r\n", p_msg->aucMessage[i]);
    }
}


/**@brief Function for handling messages received over
 * the ANT BLAZE network
 * Runs in app context (same context as ant_blaze_gateway_process_channel_event)
 * @param rx_msg            Message received
 */
static void rx_msg_handler(ant_blaze_message_t rx_msg)
{
    SERIAL_MESSAGE msg_to_send;
    NRF_LOG_INFO("Rx mesh message - Index: %d   Address: %d\r\n", rx_msg.index, rx_msg.address);

    if(rx_msg.length == 0)
    {
        // Nothing to send
        return;
    }

    // Encode as ANT extended serial message
    msg_to_send.SERIAL_MESSAGE_ucSize = rx_msg.length + MESG_GATEWAY_RX_EVENT_SIZE;
    msg_to_send.SERIAL_MESSAGE_ucMesgID = (uint8_t) (MESG_EXT_RESPONSE_ID >> 8);
    msg_to_send.SERIAL_MESSAGE_ucSubID = (uint8_t) (MESG_EXT_RESPONSE_ID & 0xFF);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[1] = (uint8_t) (ANT_BLAZE_GATEWAY_RX_EVENT >> 8);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[2] = (uint8_t) (ANT_BLAZE_GATEWAY_RX_EVENT & 0xFF);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[3] = ANT_BLAZE_GATEWAY_RX_MESG;
    msg_to_send.SERIAL_MESSAGE_aucMesgData[4] = (uint8_t) rx_msg.address;
    msg_to_send.SERIAL_MESSAGE_aucMesgData[5] = (uint8_t) (rx_msg.address >> 8);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[6] = (uint8_t) rx_msg.index;
    msg_to_send.SERIAL_MESSAGE_aucMesgData[7] = (uint8_t) (rx_msg.index >> 8);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[8] = (uint8_t) (rx_msg.index >> 16);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[9] = (uint8_t) (rx_msg.index >> 24);
    msg_to_send.SERIAL_MESSAGE_aucMesgData[10] = rx_msg.length;
    memcpy(&msg_to_send.SERIAL_MESSAGE_aucMesgData[11], rx_msg.p_data, rx_msg.length);

    send_serial_message(&msg_to_send);

    NRF_LOG_INFO("Serial Message Process TX:\r\n");
    NRF_LOG_HEXDUMP_INFO(msg_to_send.aucMessage, msg_to_send.SERIAL_MESSAGE_ucSize + 2);
}


/** @brief The main function
 */
int main(void)
{
    uint32_t err_code;

    m_rx_message.SERIAL_MESSAGE_ucSize = 0;
    m_tx_message.SERIAL_MESSAGE_ucSize = 0;

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    utils_init();
    uart_init();

    softdevice_init();

    err_code = ant_blaze_gateway_init(rx_msg_handler, NULL, app_error_handler, ANT_BLAZE_EVALUATION_LICENSE_KEY);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("ANT BLAZE Gateway Network Processor Initialized\r\n");

    // Enter main loop
    for (;;)
    {
        app_sched_execute();

        if(m_tx_message.SERIAL_MESSAGE_ucSize != 0)
        {
            send_serial_message(&m_tx_message);
            m_tx_message.SERIAL_MESSAGE_ucSize = 0;
        }

        NRF_LOG_FLUSH();

        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}
