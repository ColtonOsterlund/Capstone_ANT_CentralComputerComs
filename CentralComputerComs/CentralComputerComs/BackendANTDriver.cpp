/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except
in compliance with this license.

Copyright (c) Dynastream Innovations Inc. 2016
All rights reserved.
*/

#define _CRT_SECURE_NO_WARNINGS 1 //this is to silence the warning about scanf not being safe - *look into this*

#include <stdio.h>
#include <assert.h>

#include "demo.h"

#include "types.h"
#include "antmessage.h"

#include "antdefines.h"

#include "libant.h"

#include "BackendANTDriver.h"

#define ENABLE_EXTENDED_MESSAGES // Un - comment to enable extended messages.

#define USER_BAUDRATE            (57600)  // For AP1, use 50000; for AT3/AP2, use 57600
#define USER_RADIOFREQ           (66)     // RF Frequency + 2400 MHz

#define USER_ANTCHANNEL          (7)      // ANT channel to use
#define USER_DEVICENUM           (0)     // Device number
#define USER_DEVICETYPE          (3)      // Device type
#define USER_TRANSTYPE           (2)      // 2 = 1-Byte Shared Address Transmission type

#define USER_NETWORK_KEY         {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}
#define USER_NETWORK_NUM         (0)      // The network key is assigned to this network number

#define USER_CHANNEL_TYPE        (0x30)   // 0x30 = Shared Transmit Channel


#define MAX_CHANNEL_EVENT_SIZE   (MESG_MAX_SIZE_VALUE)     // Channel event buffer size, assumes worst case extended message size
#define MAX_RESPONSE_SIZE        (MESG_MAX_SIZE_VALUE)     // Protocol response buffer size


// Indexes into message recieved from ANT
#define MESSAGE_BUFFER_DATA1_INDEX ((UCHAR) 0)
#define MESSAGE_BUFFER_DATA2_INDEX ((UCHAR) 1)
#define MESSAGE_BUFFER_DATA3_INDEX ((UCHAR) 2)
#define MESSAGE_BUFFER_DATA4_INDEX ((UCHAR) 3)
#define MESSAGE_BUFFER_DATA5_INDEX ((UCHAR) 4)
#define MESSAGE_BUFFER_DATA6_INDEX ((UCHAR) 5)
#define MESSAGE_BUFFER_DATA7_INDEX ((UCHAR) 6)
#define MESSAGE_BUFFER_DATA8_INDEX ((UCHAR) 7)
#define MESSAGE_BUFFER_DATA9_INDEX ((UCHAR) 8)
#define MESSAGE_BUFFER_DATA10_INDEX ((UCHAR) 9)
#define MESSAGE_BUFFER_DATA11_INDEX ((UCHAR) 10)
#define MESSAGE_BUFFER_DATA12_INDEX ((UCHAR) 11)
#define MESSAGE_BUFFER_DATA13_INDEX ((UCHAR) 12)
#define MESSAGE_BUFFER_DATA14_INDEX ((UCHAR) 13)



// Local Variables
static UCHAR aucChannelBuffer[MAX_CHANNEL_EVENT_SIZE];
static UCHAR aucResponseBuffer[MAX_RESPONSE_SIZE];
static UCHAR aucTransmitBuffer[ANT_STANDARD_DATA_PAYLOAD_SIZE];
static UCHAR ucDeviceNumber;
static bool bBroadcasting;
static bool bDisplay;
static bool timed_out = false;

static UCHAR current_msg_buffer[ANT_STANDARD_DATA_PAYLOAD_SIZE];
static ANTServer* backend_server = nullptr;

// Declare callback functions into the library.
static BOOL ANT_MessageProtocol_Callback(UCHAR ucChannel_, UCHAR ucEvent_);
static BOOL ANT_DLL_Serial_Callback(UCHAR ucChannel_, UCHAR ucMessageId_);


//Declare functions for our application
void Program_Init(UCHAR ucDeviceNumber_);
void Program_Start();
void Retry_ACK_send();

void Set_timeout() {
    timed_out = true;
}

void Set_backend_ANT_server(ANTServer* server) {
    backend_server = server;
}

void Send_message_to_ANT(unsigned char* msg, int length) {
    for (int i = 0; i < length; i++) {
        current_msg_buffer[i] = msg[i];
    }
    ANT_SendAcknowledgedData(USER_ANTCHANNEL, current_msg_buffer);
}

void Retry_ACK_send() {
    //ANT_SendAcknowledgedData(USER_ANTCHANNEL, current_msg_buffer);
}

void Run_driver(unsigned char device_number)
{
    if (backend_server == nullptr) {
        printf("ANT backend server not set");
        return;
    }
    //ucDeviceNumber = 0xFF; //This indicates which of the plugged in USB ANT devices you'd like to use - starting at 0
    ucDeviceNumber = device_number;
    Program_Init(ucDeviceNumber);
    Program_Start();
}



////////////////////////////////////////////////////////////////////////////////
// Program_Init
//
// Initialize program parameters.
//
// ucDeviceNumber_: USB Device Number (0 for first USB stick plugged and so on)
//                  If not specified on command line, 0xFF is passed in as invalid.
//
////////////////////////////////////////////////////////////////////////////////
void Program_Init(UCHAR ucDeviceNumber_)
{

    BOOL bStatus;
    bBroadcasting = true; //turn on broadcasting
    bDisplay = true; //turn on console message display

    // Load the ANT DLL functions.
    if (!ANT_Load())
    {
        printf("Failed to load ANT Library\n");
        exit(0);
    }

    // Get library version
    if (ANT_LibVersionSupport())
    {
        printf("ANT Library Version %s\n", ANT_LibVersion());
    }


    // The device number depends on how many USB sticks have been
    // plugged into the PC. The first USB stick plugged will be 0
    // the next 1 and so on.
    //
    // The Baud Rate depends on the ANT solution being used. AP1
    // is 50000, all others are 57600
    bStatus = ANT_Init(ucDeviceNumber_, USER_BAUDRATE);
    assert(bStatus);

    // Assign callback functions. One for serial message responses
    // and the other for channel events. Each channel event can have
    // its own callback function defined. Since we are only
    // going to open one channel, setup one callback function
    // for the channel callback
    ANT_AssignResponseFunction(ANT_DLL_Serial_Callback, aucResponseBuffer); //Serial callback
    ANT_AssignChannelEventFunction(USER_ANTCHANNEL, ANT_MessageProtocol_Callback, aucChannelBuffer); //Channel callback

    printf("Initialization was successful!\n"); fflush(stdout);


    return;
}

////////////////////////////////////////////////////////////////////////////////
// Program_Start
//
// Start the program.
//
////////////////////////////////////////////////////////////////////////////////
void Program_Start()
{
    // Reset system
    printf("Resetting module...\n");
    ANT_ResetSystem();         // Soft reset
    ANT_Nap(1000);             // Need to delay after a reset

    // Set Network key to start of ANT initialization
    printf("Setting network key...\n");
    UCHAR ucNetKey[8] = USER_NETWORK_KEY;
    ANT_SetNetworkKey(USER_NETWORK_NUM, ucNetKey);

    while (!backend_server->is_terminated())
    {
        // Should we do something in here
        if (timed_out) {
            timed_out = false;
            backend_server->pending_message_timed_out();
        }
        ANT_Nap(0);
    }

    // Clean up ANT
    printf("Disconnecting module...\n");
    ANT_Close();

    printf("Program has completed successfully!\n");

    return;
}



////////////////////////////////////////////////////////////////////////////////
// ANT_MessageProtocol_Callback
//
// Callback function passed to ANT DLL. Called whenever a channel event
// is recieved.
//
// ucChannel_: Channel number of event
// ucEvent_: Channel event as described in section 9.5.6 of the messaging document
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_MessageProtocol_Callback(UCHAR ucChannel_, UCHAR ucEvent_)
{

    BOOL bPrintBuffer = FALSE;
    UCHAR ucDataOffset = MESSAGE_BUFFER_DATA2_INDEX;   // For most data messages

    switch (ucEvent_)
    {
    case EVENT_TX:
    {
        // This event indicates that a message has just been
        // sent over the air. We take advantage of this event to set
        // up the data for the next message period.
        //static UCHAR ucIncrement = 0;      // Increment the first byte of the buffer

       // aucTransmitBuffer[0] = ucIncrement++;

        // Broadcast data will be sent over the air on
        // the next message period.
        if (bBroadcasting)
        {
            ANT_SendBroadcastData(USER_ANTCHANNEL, aucTransmitBuffer);

            // Echo what the data will be over the air on the next message period.
            if (bDisplay)
            {
                if (aucTransmitBuffer[MESSAGE_BUFFER_DATA1_INDEX] != 0) {
                    printf("Tx:(%d): [%02x],[%02x],[%02x],[%02x],[%02x],[%02x],[%02x],[%02x]\n",
                        USER_ANTCHANNEL,
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA1_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA2_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA3_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA4_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA5_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA6_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA7_INDEX],
                        aucTransmitBuffer[MESSAGE_BUFFER_DATA8_INDEX]);
                }
            }
            else
            {
                static int iIndex = 0;
                static char ac[] = { '|','/','-','\\' };
                printf("Tx: %c\r", ac[iIndex++]); fflush(stdout);
                iIndex &= 3;
            }
        }

        break;
    }
    case EVENT_RX_FLAG_ACKNOWLEDGED:
    case EVENT_RX_FLAG_BURST_PACKET:
    case EVENT_RX_FLAG_BROADCAST:
    {
        // This is an event generated by the DLL. It is not strictly
        // returned by the ANT part. To enable, must call ANT_RxExtMesgsEnable first.

        // This event has a flag at the end of the data payload
        // that indicates additional information. Process this here
        // and then fall-through to process the data payload.


        UCHAR ucFlag = aucChannelBuffer[MESSAGE_BUFFER_DATA10_INDEX];

        if (bDisplay && ucFlag & ANT_EXT_MESG_BITFIELD_DEVICE_ID)
        {
            // Channel ID of the device that we just recieved a message from.
            USHORT usDeviceNumber = aucChannelBuffer[MESSAGE_BUFFER_DATA11_INDEX] | (aucChannelBuffer[MESSAGE_BUFFER_DATA12_INDEX] << 8);
            UCHAR ucDeviceType = aucChannelBuffer[MESSAGE_BUFFER_DATA13_INDEX];
            UCHAR ucTransmissionType = aucChannelBuffer[MESSAGE_BUFFER_DATA14_INDEX];

            printf("Chan ID(%d/%d/%d) - ", usDeviceNumber, ucDeviceType, ucTransmissionType);
        }
        // INTENTIONAL FALLTHROUGH
    }
    case EVENT_RX_ACKNOWLEDGED:
    case EVENT_RX_BURST_PACKET:
    case EVENT_RX_BROADCAST:
    {
        // This is an event generated by the DLL. It is not strictly
        // returned by the ANT part.

        // Display recieved message
        bPrintBuffer = TRUE;
        ucDataOffset = MESSAGE_BUFFER_DATA2_INDEX;   // For most data messages

        if (bDisplay)
        {
            if (ucEvent_ == EVENT_RX_ACKNOWLEDGED || ucEvent_ == EVENT_RX_FLAG_ACKNOWLEDGED) {
                printf("Acked Rx:(%d): \n", aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
                backend_server->receive_ANT_message(&aucChannelBuffer[ucDataOffset]);
            }
            else if (ucEvent_ == EVENT_RX_BURST_PACKET || ucEvent_ == EVENT_RX_FLAG_BURST_PACKET)
                printf("Burst(0x%02x) Rx:(%d): ", ((aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX] & 0xE0) >> 5), aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX] & 0x1F);
            else
                printf("Rx:(%d): ", aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
        }
        break;
    }

    case EVENT_RX_EXT_ACKNOWLEDGED:
    case EVENT_RX_EXT_BURST_PACKET:
    case EVENT_RX_EXT_BROADCAST:
    {
        // This is an event generated by the DLL. It is not strictly
        // returned by the ANT part. To enable, must call ANT_RxExtMesgsEnable first.

        // The "extended" part of this message is the 4-byte channel
        // id of the device that we recieved this message from. This event
        // is only available on the AT3. The AP2 uses the EVENT_RX_FLAG_BROADCAST
        // as shown above.

        // Channel ID of the device that we just recieved a message from.
        USHORT usDeviceNumber = aucChannelBuffer[MESSAGE_BUFFER_DATA2_INDEX] | (aucChannelBuffer[MESSAGE_BUFFER_DATA3_INDEX] << 8);
        UCHAR ucDeviceType = aucChannelBuffer[MESSAGE_BUFFER_DATA4_INDEX];
        UCHAR ucTransmissionType = aucChannelBuffer[MESSAGE_BUFFER_DATA5_INDEX];

        bPrintBuffer = TRUE;
        ucDataOffset = MESSAGE_BUFFER_DATA6_INDEX;   // For most data messages

        if (bDisplay)
        {

            // Display the channel id
            printf("Chan ID(%d/%d/%d) ", usDeviceNumber, ucDeviceType, ucTransmissionType);

            if (ucEvent_ == EVENT_RX_EXT_ACKNOWLEDGED)
                printf("- Acked Rx:(%d): ", aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
            else if (ucEvent_ == EVENT_RX_EXT_BURST_PACKET)
                printf("- Burst(0x%02x) Rx:(%d): ", ((aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX] & 0xE0) >> 5), aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX] & 0x1F);
            else
                printf("- Rx:(%d): ", aucChannelBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
        }

        break;
    }
    case EVENT_RX_SEARCH_TIMEOUT:
    {
        printf("Search Timeout\n");
        break;
    }
    case EVENT_RX_FAIL:
    {
        printf("Rx Fail\n");
        break;
    }
    case EVENT_TRANSFER_RX_FAILED:
    {
        printf("Burst receive has failed\n");
        break;
    }
    case EVENT_TRANSFER_TX_COMPLETED:
    {
        printf("Tranfer Completed.\n");
        break;
    }
    case EVENT_TRANSFER_TX_FAILED:
    {
        printf("Tranfer Failed. Retrying...\n");
        Retry_ACK_send();
        break;
    }
    case EVENT_CHANNEL_CLOSED:
    {
        // This event should be used to determine that the channel is closed.
        printf("Channel Closed\n");
        printf("Unassigning Channel...\n");
        ANT_UnAssignChannel(USER_ANTCHANNEL);
        break;
    }
    case EVENT_RX_FAIL_GO_TO_SEARCH:
    {
        printf("Go to Search.\n");
        break;
    }
    case EVENT_CHANNEL_COLLISION:
    {
        printf("Channel Collision\n");
        break;
    }
    case EVENT_TRANSFER_TX_START:
    {
        printf("Burst Started\n");
        break;
    }
    default:
    {
        printf("Unhandled channel event 0x%X\n", ucEvent_);
        break;
    }

    }

    // If we recieved a data message, diplay its contents here.
    if (bPrintBuffer)
    {
        if (bDisplay)
        {
            printf("[%02x],[%02x],[%02x],[%02x],[%02x],[%02x],[%02x],[%02x]\n",
                aucChannelBuffer[ucDataOffset + 0],
                aucChannelBuffer[ucDataOffset + 1],
                aucChannelBuffer[ucDataOffset + 2],
                aucChannelBuffer[ucDataOffset + 3],
                aucChannelBuffer[ucDataOffset + 4],
                aucChannelBuffer[ucDataOffset + 5],
                aucChannelBuffer[ucDataOffset + 6],
                aucChannelBuffer[ucDataOffset + 7]);

        }
        else
        {
            static int iIndex = 0;
            static char ac[] = { '|','/','-','\\' };
            printf("Rx: %c\r", ac[iIndex++]); fflush(stdout);
            iIndex &= 3;

        }
    }

    return(TRUE);
}



////////////////////////////////////////////////////////////////////////////////
// Test_ResponseCallback
//
// Callback function passed to ANT DLL. Called whenever a message is recieved from ANT
// unless that message is a channel event message.
//
// ucChannel_: Channel number of relevent to the message
// ucMessageId_: Message ID of the message recieved from ANT.
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_DLL_Serial_Callback(UCHAR ucChannel_, UCHAR ucMessageId_)
{
    BOOL bSuccess = FALSE;

    switch (ucMessageId_)
    {

    case MESG_STARTUP_MESG_ID:
    {
        printf("RESET Complete, reason: ");

        UCHAR ucReason = aucResponseBuffer[MESSAGE_BUFFER_DATA1_INDEX];

        if (ucReason == RESET_POR)
            printf("RESET_POR");
        if (ucReason & RESET_SUSPEND)
            printf("RESET_SUSPEND ");
        if (ucReason & RESET_SYNC)
            printf("RESET_SYNC ");
        if (ucReason & RESET_CMD)
            printf("RESET_CMD ");
        if (ucReason & RESET_WDT)
            printf("RESET_WDT ");
        if (ucReason & RESET_RST)
            printf("RESET_RST ");
        printf("\n");

        break;
    }
    case MESG_CAPABILITIES_ID:
    {
        printf("CAPABILITIES:\n");
        printf("   Max ANT Channels: %d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
        printf("   Max ANT Networks: %d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX]);

        UCHAR ucStandardOptions = aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX];
        UCHAR ucAdvanced = aucResponseBuffer[MESSAGE_BUFFER_DATA4_INDEX];
        UCHAR ucAdvanced2 = aucResponseBuffer[MESSAGE_BUFFER_DATA5_INDEX];

        printf("Standard Options:\n");
        if (ucStandardOptions & CAPABILITIES_NO_RX_CHANNELS)
            printf("CAPABILITIES_NO_RX_CHANNELS\n");
        if (ucStandardOptions & CAPABILITIES_NO_TX_CHANNELS)
            printf("CAPABILITIES_NO_TX_CHANNELS\n");
        if (ucStandardOptions & CAPABILITIES_NO_RX_MESSAGES)
            printf("CAPABILITIES_NO_RX_MESSAGES\n");
        if (ucStandardOptions & CAPABILITIES_NO_TX_MESSAGES)
            printf("CAPABILITIES_NO_TX_MESSAGES\n");
        if (ucStandardOptions & CAPABILITIES_NO_ACKD_MESSAGES)
            printf("CAPABILITIES_NO_ACKD_MESSAGES\n");
        if (ucStandardOptions & CAPABILITIES_NO_BURST_TRANSFER)
            printf("CAPABILITIES_NO_BURST_TRANSFER\n");

        printf("Advanced Options:\n");
        if (ucAdvanced & CAPABILITIES_OVERUN_UNDERRUN)
            printf("CAPABILITIES_OVERUN_UNDERRUN\n");
        if (ucAdvanced & CAPABILITIES_NETWORK_ENABLED)
            printf("CAPABILITIES_NETWORK_ENABLED\n");
        if (ucAdvanced & CAPABILITIES_AP1_VERSION_2)
            printf("CAPABILITIES_AP1_VERSION_2\n");
        if (ucAdvanced & CAPABILITIES_SERIAL_NUMBER_ENABLED)
            printf("CAPABILITIES_SERIAL_NUMBER_ENABLED\n");
        if (ucAdvanced & CAPABILITIES_PER_CHANNEL_TX_POWER_ENABLED)
            printf("CAPABILITIES_PER_CHANNEL_TX_POWER_ENABLED\n");
        if (ucAdvanced & CAPABILITIES_LOW_PRIORITY_SEARCH_ENABLED)
            printf("CAPABILITIES_LOW_PRIORITY_SEARCH_ENABLED\n");
        if (ucAdvanced & CAPABILITIES_SCRIPT_ENABLED)
            printf("CAPABILITIES_SCRIPT_ENABLED\n");
        if (ucAdvanced & CAPABILITIES_SEARCH_LIST_ENABLED)
            printf("CAPABILITIES_SEARCH_LIST_ENABLED\n");

        printf("Advanced 2 Options 1:\n");
        if (ucAdvanced2 & CAPABILITIES_LED_ENABLED)
            printf("CAPABILITIES_LED_ENABLED\n");
        if (ucAdvanced2 & CAPABILITIES_EXT_MESSAGE_ENABLED)
            printf("CAPABILITIES_EXT_MESSAGE_ENABLED\n");
        if (ucAdvanced2 & CAPABILITIES_SCAN_MODE_ENABLED)
            printf("CAPABILITIES_SCAN_MODE_ENABLED\n");
        if (ucAdvanced2 & CAPABILITIES_RESERVED)
            printf("CAPABILITIES_RESERVED\n");
        if (ucAdvanced2 & CAPABILITIES_PROX_SEARCH_ENABLED)
            printf("CAPABILITIES_PROX_SEARCH_ENABLED\n");
        if (ucAdvanced2 & CAPABILITIES_EXT_ASSIGN_ENABLED)
            printf("CAPABILITIES_EXT_ASSIGN_ENABLED\n");
        if (ucAdvanced2 & CAPABILITIES_FS_ANTFS_ENABLED)
            printf("CAPABILITIES_FREE_1\n");
        if (ucAdvanced2 & CAPABILITIES_FIT1_ENABLED)
            printf("CAPABILITIES_FIT1_ENABLED\n");

        break;
    }
    case MESG_CHANNEL_STATUS_ID:
    {
        printf("Got Status\n");

        char astrStatus[][32] = { "STATUS_UNASSIGNED_CHANNEL",
                                   "STATUS_ASSIGNED_CHANNEL",
                                   "STATUS_SEARCHING_CHANNEL",
                                   "STATUS_TRACKING_CHANNEL" };

        UCHAR ucStatusByte = aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX] & STATUS_CHANNEL_STATE_MASK; // MUST MASK OFF THE RESERVED BITS
        printf("STATUS: %s\n", astrStatus[ucStatusByte]);
        break;
    }
    case MESG_CHANNEL_ID_ID:
    {
        // Channel ID of the device that we just recieved a message from.
        USHORT usDeviceNumber = aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX] | (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] << 8);
        UCHAR ucDeviceType = aucResponseBuffer[MESSAGE_BUFFER_DATA4_INDEX];
        UCHAR ucTransmissionType = aucResponseBuffer[MESSAGE_BUFFER_DATA5_INDEX];

        printf("CHANNEL ID: (%d/%d/%d)\n", usDeviceNumber, ucDeviceType, ucTransmissionType);
        break;
    }
    case MESG_VERSION_ID:
    {
        printf("VERSION: %s\n", (char*)&aucResponseBuffer[MESSAGE_BUFFER_DATA1_INDEX]);
        break;
    }
    case MESG_RESPONSE_EVENT_ID:
    {
        switch (aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX])
        {
        case MESG_NETWORK_KEY_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error configuring network key: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Network key set\n");
            printf("Assigning channel...\n");
             
            if (1) //MASTER
            {
                bSuccess = ANT_AssignChannel(USER_ANTCHANNEL, PARAMETER_TX_NOT_RX, USER_NETWORK_NUM);
            }
            else if (0) //SLAVE
            {
                bSuccess = ANT_AssignChannel(USER_ANTCHANNEL, 0, USER_NETWORK_NUM);
            }
            break;
        }

        case MESG_ASSIGN_CHANNEL_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error assigning channel: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Channel assigned\n");
            printf("Setting Channel ID...\n");
            bSuccess = ANT_SetChannelId(USER_ANTCHANNEL, USER_DEVICENUM, USER_DEVICETYPE, USER_TRANSTYPE);
            break;
        }

        case MESG_CHANNEL_ID_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error configuring Channel ID: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Channel ID set\n");
            printf("Setting Radio Frequency...\n");
            bSuccess = ANT_SetChannelRFFreq(USER_ANTCHANNEL, USER_RADIOFREQ);
            break;
        }

        case MESG_CHANNEL_RADIO_FREQ_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error configuring Radio Frequency: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Radio Frequency set\n");
            printf("Opening channel...\n");
            bBroadcasting = TRUE;
            bSuccess = ANT_OpenChannel(USER_ANTCHANNEL);
            break;
        }

        case MESG_OPEN_CHANNEL_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error opening channel: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                bBroadcasting = FALSE;
                break;
            }
            printf("Channel opened\n");
#if defined (ENABLE_EXTENDED_MESSAGES)
            printf("Enabling extended messages...\n");
            ANT_RxExtMesgsEnable(TRUE);
#endif
            break;
        }

        case MESG_RX_EXT_MESGS_ENABLE_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] == INVALID_MESSAGE)
            {
                printf("Extended messages not supported in this ANT product\n");
                break;
            }
            else if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error enabling extended messages: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Extended messages enabled\n");
            break;
        }
        case MESG_UNASSIGN_CHANNEL_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error unassigning channel: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            printf("Channel unassigned\n");
            break;
        }
        case MESG_CLOSE_CHANNEL_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] == CHANNEL_IN_WRONG_STATE)
            {
                // We get here if we tried to close the channel after the search timeout (slave)
                printf("Channel is already closed\n");
                printf("Unassigning channel...\n");
                ANT_UnAssignChannel(USER_ANTCHANNEL);
                break;
            }
            else if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] != RESPONSE_NO_ERROR)
            {
                printf("Error closing channel: Code 0%d\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX]);
                break;
            }
            // If this message was successful, wait for EVENT_CHANNEL_CLOSED to confirm the channel is closed
            break;
        }
        case MESG_REQUEST_ID:
        {
            if (aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX] == INVALID_MESSAGE)
            {
                printf("Requested message not supported in this ANT product\n");
                break;
            }
        }
        case TRANSFER_IN_PROGRESS:
            printf("ACK message in progress");


        default:
        {
            printf("Unhandled response %d to message 0x%X\n", aucResponseBuffer[MESSAGE_BUFFER_DATA3_INDEX], aucResponseBuffer[MESSAGE_BUFFER_DATA2_INDEX]);
            break;
        }
        }
    }
    }
    return(TRUE);
}
