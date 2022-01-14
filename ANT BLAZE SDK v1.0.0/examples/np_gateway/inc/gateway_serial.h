/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2017
All rights reserved.
*/

#ifndef __GATEWAY_SERIAL_H__
#define __GATEWAY_SERIAL_H__

#include <stdint.h>

// The serialization for the gateway interface is based on ANT extended serial messages.
// Refer to the "Interfacing with ANT General Purpose Chipsets and Modules" for details on serial framing
// for ANT messages, and the "Integrated ANT-FS Interface Control Document" for more details on extended
// serial messaging.

// Extended-ID format for ANT request message. This application processes requests for ANT capabilities only.
#define ANT_REQUEST                     ((uint16_t) 0x4D00)

/////////// Commands ////////////////

// Serialization for ant_blaze_gateway_config()
// Multi-byte fields are encoded as little endian
// Format:
// Byte(s)  Field
// 0        0xE2 (extended command)
// 1        0xB0 (extended command: config)
// 2-3      Node ID
// 4-5      Network ID
// 6-7      Channel Period
// 8        Tx Power
// 9        Bits 0-1: Number of beacons (1-3)
//          Bits 2:   Encryption enabled (0=disabled, 1 = enabled)
//          Bits 3-7: Reserved
// 10       Radio Frequency 1
// 11       Radio Frequency 2. Set to zero if not used.
// 12       Radio Frequency 3. Set to zero if not used.
// 13-20    Network Key
// 21-36    Encryption key. Fill with zeroes if not used.
#define ANT_BLAZE_GATEWAY_CONFIG               ((uint16_t) 0xE2B0)

// Serialization for ant_blaze_gateway_start()
// Format:
// Byte(s)  Field
// 0        0xE2  (extended command)
// 1        0xB1  (extended command: start)
#define ANT_BLAZE_GATEWAY_START                ((uint16_t) 0xE2B1)

// Serialization for ant_blaze_gateway_stop()
// Format:
// Byte(s)  Field
// 0        0xE2  (extended command)
// 1        0xB2  (extended command: stop)
#define ANT_BLAZE_GATEWAY_STOP                 ((uint16_t) 0xE2B2)

// Serialization for ant_blaze_gateway_send_message()
// Multi-byte fields are encoded as little endian
// SEND_RESPONSE_ON_SEND can be used to enable/disable responses
// to this message to reduce serial traffic when handling a large
// amount of messages
// Format:
// Byte(s)  Field
// 0        0xE2  (extended command)
// 1        0xB3  (extended command: send message)
// 2-3      Address
// 4        Length
// 5+       Data
#define ANT_BLAZE_GATEWAY_SEND_MESSAGE         ((uint16_t) 0xE2B3)

// The return value of calling commands is encoded as follows
// Format:
// Byte(s)  Field
// 0        0xE0  (extended response/event)
// 1        0x00  (extended response/event)
// 2        0xE2  (extended ID being responded to)
// 3        0xB?  (extended ID being responded to)
// 4        Return Code

/////////// Requested Messages ////////////////

// To request information from  the library (e.g., version string), use the following message
// Format:
// Byte(s)  Field
// 0        0xE1
// 1        0x00
// 2        Requested message (e.g. 0xE2)
// 3        Requested message (e.g. 0xC0)

// The response for a request for version string is encoded as follows.
// Format:
// Byte(s)  Field
// 0        0xE0  (extended response/event)
// 1        0x00  (extended response/event)
// 2        0xE2  (extended ID being responded to)
// 3        0xC0  (extended ID being responded to)
// 4+       Version string (actual length of the entire message encoded in the ANT message size field)
#define ANT_BLAZE_GATEWAY_GET_VERSION_STRING   ((uint16_t) 0xE2C0)

// The response for a request for the current scan frequency is encoded as follows.
// Format:
// Byte(s)  Field
// 0        0xE0  (extended response/event)
// 1        0x00  (extended response/event)
// 2        0xE2  (extended ID being responded to)
// 3        0xC1  (extended ID being responded to)
// 4        Active Scan Frequency
#define ANT_BLAZE_GATEWAY_GET_SCAN_FREQ        ((uint16_t) 0xE2C1)

// The response for a request for the number of nodes in range is encoded as follows.
// Format:
// Byte(s)  Field
// 0        0xE0  (extended response/event)
// 1        0x00  (extended response/event)
// 2        0xE2  (extended ID being responded to)
// 3        0xC2  (extended ID being responded to)
// 4        Number of nodes in range
#define ANT_BLAZE_GATEWAY_GET_NODES_IN_RANGE   ((uint16_t) 0xE2C2)

/////////// Events ////////////////
#define ANT_BLAZE_GATEWAY_RX_EVENT             ((uint16_t) 0xE230)

// Messages received over the mesh network are serialized using the following message format
// Multi-byte fields arfe little endian
// Format:
// Byte(s)  Field
// 0        0xE0  (extended response/event)
// 1        0x00  (extended response/event)
// 2        0xE2  (extended ID being responded to - RX event)
// 3        0x30  (extended ID being responded to - Rx event)
// 4        0x01  (event code - Rx message from mesh)
// 5-6      Address
// 7-10     Index
// 11       Payload Length
// 12+      Data Payload
#define ANT_BLAZE_GATEWAY_RX_MESG              ((uint8_t) 0x01)

// Message sizes
#define MESG_EXTENDED_RESPONSE_SIZE     ((uint8_t) 4)
#define MESG_GATEWAY_SEND_BASE_SIZE     ((uint8_t) 4)
#define MESG_GATEWAY_CONFIG_SIZE        ((uint8_t) 36)
#define MESG_GATEWAY_VERSION_BASE_SIZE  ((uint8_t) 3)
#define MESG_GATEWAY_GET_SCAN_FREQ_SIZE ((uint8_t) 3)
#define MESG_GATEWAY_GET_NODES_IN_RANGE_SIZE    ((uint8_t) 4)
#define MESG_GATEWAY_RX_EVENT_SIZE      ((uint8_t) 11)

// Bit masks
#define NUM_CHANNELS_MASK               ((uint8_t) 0x03)
#define NUM_CHANNELS_SHIFT              ((uint8_t) 0x00)
#define ENCRYPTION_MODE_MASK            ((uint8_t) 0x0C)
#define ENCRYPTION_MODE_SHIFT           ((uint8_t) 0x02)

#define ENCRYPTION_OFF                  ((uint8_t) 0x00)
#define ENCRYPTION_PAYLOAD              ((uint8_t) 0x01)

#define SERIAL_MESG_MAX_DATA_SIZE       ((uint8_t) 50)  // Maximum message length (MESG_GATEWAY_RX_EVENT_SIZE + ANT_BLAZE_MAX_MESSAGE_LENGTH - 1 (Sub ID))
#define SERIAL_MESG_MAX_SIZE_VALUE      (SERIAL_MESG_MAX_DATA_SIZE + 1) // this is the maximum value that the serial message size value is allowed to be (Data + Sub ID)
#define SERIAL_MESG_BUFFER_SIZE         (SERIAL_MESG_MAX_DATA_SIZE + 4) // Message buffer size (Max Data + Size + Msg ID + Msg Sub ID + Checksum)
#define SERIAL_MESG_FRAMED_SIZE         (SERIAL_MESG_MAX_DATA_SIZE + 2) // Framed message size (Max Data + Msg ID + Msg Sub ID)

/**
 * @brief The structure that holds serial messages
 */
typedef union SERIAL_MESSAGE
{
   uint32_t ulForceAlign; // force the struct to be 4-byte aligned
   uint8_t aucMessage[SERIAL_MESG_BUFFER_SIZE]; // the complete message buffer
   struct
   {
      uint8_t ucSize; // the message size
      union
      {
         uint8_t aucFramedData[SERIAL_MESG_FRAMED_SIZE]; // pointer for serial framer
         struct
         {
            uint8_t ucMesgID; // the message ID
            union
            {
               uint8_t aucMesgData[SERIAL_MESG_MAX_SIZE_VALUE]; // the message data
               struct
               {
                  uint8_t ucSubID; // subID portion of ext ID message
                  uint8_t aucData[SERIAL_MESG_MAX_DATA_SIZE]; // Serial message data
               }stMesgData;
            }uMesgData;
         }stFramedData;
      }uFramedData;
      uint8_t ucCheckSum; // the message checksum
   }stMessage;
} SERIAL_MESSAGE;

#define SERIAL_MESSAGE_ucSize               stMessage.ucSize
#define SERIAL_MESSAGE_ucMesgID             stMessage.uFramedData.stFramedData.ucMesgID
#define SERIAL_MESSAGE_ucSubID              stMessage.uFramedData.stFramedData.uMesgData.stMesgData.ucSubID
#define SERIAL_MESSAGE_aucMesgData          stMessage.uFramedData.stFramedData.uMesgData.aucMesgData
#define SERIAL_MESSAGE_ucCheckSum           stMessage.ucCheckSum
#define SERIAL_MESSAGE_aucFramedData        stMessage.uFramedData.aucFramedData


/**@brief Gateway serial message handler
 *
 * This function handles messages received over the serial interface and calls the corresponding function in
 * the ANT BLAZE Gateway Library. If a response is received from the library as a result of
 * the call, it is populated in the tx msg so it can be transmitted back over the serial
 * interface.
 *
 * @param[in] p_rx_msg  Message received over the serial interface, to be passed to the library
 * @param[in] p_tx_msg  Response message received from the library to be transmitted over the serial interface, if available
 */
void gateway_serial_message_process(SERIAL_MESSAGE *p_rx_msg, SERIAL_MESSAGE *p_tx_msg);

#endif
