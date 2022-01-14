/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2017
All rights reserved.
*/

#include <string.h>
#include "sdk_config.h"
#include "gateway_serial.h"
#include "ant_parameters.h"
#include "ant_interface.h"
#include "nrf_error.h"
#include "ant_blaze_gateway_interface.h"
#include "nrf_log.h"

void gateway_serial_message_process(SERIAL_MESSAGE *p_rx_msg, SERIAL_MESSAGE *p_tx_msg)
{
    uint32_t err_code;
    uint16_t extended_msg_id = ((uint16_t) (p_rx_msg->SERIAL_MESSAGE_ucMesgID) << 8) | p_rx_msg->SERIAL_MESSAGE_ucSubID;

    NRF_LOG_INFO("Serial Message Process RX:\r\n");
    NRF_LOG_HEXDUMP_INFO(p_rx_msg->aucMessage, p_rx_msg->SERIAL_MESSAGE_ucSize + 2);

    // Setup tentative response
    // Extended ID Response
    p_tx_msg->SERIAL_MESSAGE_ucMesgID = (uint8_t) (MESG_EXT_RESPONSE_ID >> 8);
    p_tx_msg->SERIAL_MESSAGE_ucSubID = (uint8_t) (MESG_EXT_RESPONSE_ID & 0xFF);
    // Extended ID being responded to
    p_tx_msg->SERIAL_MESSAGE_aucMesgData[1] = p_rx_msg->SERIAL_MESSAGE_ucMesgID;
    p_tx_msg->SERIAL_MESSAGE_aucMesgData[2] = p_rx_msg->SERIAL_MESSAGE_ucSubID;
    // Message size for extended response
    p_tx_msg->SERIAL_MESSAGE_ucSize = MESG_EXTENDED_RESPONSE_SIZE;

    switch(extended_msg_id)
    {
        case ANT_REQUEST:
        {
            if(p_rx_msg->SERIAL_MESSAGE_aucMesgData[1] == MESG_CAPABILITIES_ID)
            {
                NRF_LOG_INFO("Rx Request for ANT Capabilities\r\n");
                err_code = sd_ant_capabilities_get(p_tx_msg->SERIAL_MESSAGE_aucMesgData);
                if (!err_code)
                {
                    p_tx_msg->SERIAL_MESSAGE_ucSize = MESG_CAPABILITIES_SIZE;
                    p_tx_msg->SERIAL_MESSAGE_ucMesgID = MESG_CAPABILITIES_ID;
                }
            }
            else
            {
                p_tx_msg->SERIAL_MESSAGE_ucSize = 0;
            }
            break;
        }
        case ANT_BLAZE_GATEWAY_CONFIG:
        {
            NRF_LOG_INFO("Rx Gateway Config Command\r\n");
            if(p_rx_msg->SERIAL_MESSAGE_ucSize >= MESG_GATEWAY_CONFIG_SIZE)
            {
                ant_blaze_gateway_config_t config;
                uint8_t ant_net_key[8] = {0};
                uint8_t crypto_key[16] = {0};

                config.p_ant_network_key = ant_net_key;
                config.p_encryption_key = crypto_key;

                // Copy parameters from serial message
                config.node_id = ((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[1]) | (((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[2]) << 8);
                config.network_id = ((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[3]) | (((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[4]) << 8);
                config.channel_period = ((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[5]) | (((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[6]) << 8);
                config.tx_power = p_rx_msg->SERIAL_MESSAGE_aucMesgData[7];
                config.num_channels = (p_rx_msg->SERIAL_MESSAGE_aucMesgData[8] & NUM_CHANNELS_MASK) >> NUM_CHANNELS_SHIFT;
                config.encryption_enabled = (p_rx_msg->SERIAL_MESSAGE_aucMesgData[8] & ENCRYPTION_MODE_MASK) >> ENCRYPTION_MODE_SHIFT;
                config.radio_freqs[0] = p_rx_msg->SERIAL_MESSAGE_aucMesgData[9];
                config.radio_freqs[1] = p_rx_msg->SERIAL_MESSAGE_aucMesgData[10];
                config.radio_freqs[2] = p_rx_msg->SERIAL_MESSAGE_aucMesgData[11];
                memcpy(ant_net_key, &p_rx_msg->SERIAL_MESSAGE_aucMesgData[12], 8);
                memcpy(crypto_key, &p_rx_msg->SERIAL_MESSAGE_aucMesgData[20], 16);

                err_code = ant_blaze_gateway_config(&config);
                p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = (uint8_t) err_code;
            }
            else
            {
                p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = NRF_ERROR_INVALID_PARAM;
            }

            break;
        }
        case ANT_BLAZE_GATEWAY_START:
        {
            NRF_LOG_INFO("Rx Gateway Start Command\r\n");
            err_code = ant_blaze_gateway_start();
            p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = (uint8_t) err_code;
            break;
        }
        case ANT_BLAZE_GATEWAY_STOP:
        {
            NRF_LOG_INFO("Rx Gateway Stop Command\r\n");
            err_code = ant_blaze_gateway_stop();
            p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = (uint8_t) err_code;
            break;
        }
        case ANT_BLAZE_GATEWAY_SEND_MESSAGE:
        {
            ant_blaze_message_t msg_to_send;
            NRF_LOG_INFO("Rx Request to Send Message to Mesh\r\n");
            msg_to_send.address = ((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[1]) | (((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[2]) << 8);
            msg_to_send.length = p_rx_msg->SERIAL_MESSAGE_aucMesgData[3];
            msg_to_send.index = 0; // Value is overwritten inside the library
            // Validate ANT message size and payload length
            if(p_rx_msg->SERIAL_MESSAGE_ucSize >= MESG_GATEWAY_SEND_BASE_SIZE + msg_to_send.length)
            {
                msg_to_send.p_data = &p_rx_msg->SERIAL_MESSAGE_aucMesgData[4];
                err_code = ant_blaze_gateway_send_message(&msg_to_send);
                p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = (uint8_t) err_code;
            }
            else
            {
                p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = (uint8_t) NRF_ERROR_INVALID_PARAM;
            }
            #if (!SEND_RESPONSE_ON_SEND)
            // Send no response
            // Invalidate the size
            p_tx_msg->SERIAL_MESSAGE_ucSize = 0;
            #endif
            break;
        }
        case MESG_EXT_REQUEST_ID:
        {
            uint16_t requested_message = ((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[2]) | (((uint16_t) p_rx_msg->SERIAL_MESSAGE_aucMesgData[1]) << 8);

            // Extended ID being responded to is the requested message
            p_tx_msg->SERIAL_MESSAGE_aucMesgData[1] = p_rx_msg->SERIAL_MESSAGE_aucMesgData[1];
            p_tx_msg->SERIAL_MESSAGE_aucMesgData[2] = p_rx_msg->SERIAL_MESSAGE_aucMesgData[2];

            switch(requested_message)
            {
                case ANT_BLAZE_GATEWAY_GET_VERSION_STRING:
                {
                    char version_string[11] = {0};
                    char* ptr;
                    int string_length = 11;

                    NRF_LOG_INFO("Rx Request for Gateway Lib Version String\r\n");
                    err_code = ant_blaze_gateway_get_version_string(version_string);
                    // Remove trailing null characters
                    ptr = strchr(version_string, 0);
                    if(ptr != NULL)
                        string_length = ptr - version_string + 1;
                    memcpy(&p_tx_msg->SERIAL_MESSAGE_aucMesgData[3], version_string, string_length);
                    p_tx_msg->SERIAL_MESSAGE_ucSize = MESG_GATEWAY_VERSION_BASE_SIZE + string_length;
                    break;
                }
                case ANT_BLAZE_GATEWAY_GET_SCAN_FREQ:
                {
                    uint8_t freq = 0xFF;
                    err_code = ant_blaze_gateway_get_current_scan_frequency(&freq);
                    NRF_LOG_INFO("Rx Request for Current Scan Frequency: %0d (%1d)\r\n", freq, err_code);
                    p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = freq;
                    p_tx_msg->SERIAL_MESSAGE_ucSize = MESG_GATEWAY_GET_SCAN_FREQ_SIZE;
                    break;
                }
                case ANT_BLAZE_GATEWAY_GET_NODES_IN_RANGE:
                {
                    uint16_t num_nodes;
                    err_code = ant_blaze_gateway_get_number_of_nodes_in_range(&num_nodes);
                    NRF_LOG_INFO("Rx Request for Number of Nodes in Range: %0d (%1d)\r\n", num_nodes, err_code);
                    p_tx_msg->SERIAL_MESSAGE_aucMesgData[3] = num_nodes & 0xFF;
                    p_tx_msg->SERIAL_MESSAGE_aucMesgData[4] = (num_nodes >> 8) & 0xFF;
                    p_tx_msg->SERIAL_MESSAGE_ucSize = MESG_GATEWAY_GET_NODES_IN_RANGE_SIZE;
                    break;
                }
            }
            break;
        }
        default:
        {
            // There is no response (i.e., no valid command was received)
            // Invalidate the size
            p_tx_msg->SERIAL_MESSAGE_ucSize = 0;
            break;
        }
    }

    if(p_tx_msg->SERIAL_MESSAGE_ucSize != 0)
    {
        NRF_LOG_INFO("Serial Message Process TX:\r\n");
        NRF_LOG_HEXDUMP_INFO(p_tx_msg->aucMessage, p_tx_msg->SERIAL_MESSAGE_ucSize + 2);
    }
}
