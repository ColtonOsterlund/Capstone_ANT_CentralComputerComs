/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2015
All rights reserved.
*/

#if defined(BLE_STACK_SUPPORT_REQD)
#include <string.h>
#include "ble_sf_service.h"
#include "nrf_error.h"
#include "ble_srv_common.h"
#include "ant_blaze_defines.h"

#define SFS_BASE_UUID                  {{0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0xA6, 0x86, 0xE5, 0x11, 0x10, 0x42, 0xC0, 0xA8, 0x42, 0xB6}} /**< Used vendor specific UUID. */
#define BLE_UUID_SFS_TX_CHARACTERISTIC 0x02                      /**< The UUID of the ANT BLAZE Message Characteristic. */
#define BLE_UUID_SFS_RX_CHARACTERISTIC 0x03                      /**< The UUID of the ANT BLAZE Message Characteristic. */

uint8_t m_tx_char_value[ANT_BLAZE_MAX_MESSAGE_LENGTH] = {0};
uint8_t m_rx_char_value[ANT_BLAZE_MAX_MESSAGE_LENGTH + 2] = {0}; // Message plus index

static void on_write(ble_sf_service_t * p_sfs, ble_gatts_evt_write_t * p_evt_write);
static uint32_t sf_message_tx_char_add(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init);
static uint32_t sf_message_rx_char_add(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init);

uint32_t ble_sf_service_init(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t ams_base_uuid = SFS_BASE_UUID;

    if ((p_sfs == NULL) || (p_sfs_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    p_sfs->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_sfs->evt_handler = p_sfs_init->evt_handler;
    p_sfs->is_notification_enabled = false;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&ams_base_uuid, &p_sfs->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_sfs->uuid_type;
    ble_uuid.uuid = BLE_UUID_SF_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_sfs->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the characteristics.
    err_code = sf_message_tx_char_add(p_sfs, p_sfs_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sf_message_rx_char_add(p_sfs, p_sfs_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

void ble_sf_service_on_ble_evt(ble_sf_service_t * p_sfs, ble_evt_t * p_ble_evt)
{
    if ((p_sfs == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_sfs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            p_sfs->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_sfs, &p_ble_evt->evt.gatts_evt.params.write);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_sf_service_update_rx_char(ble_sf_service_t * p_sfs, ant_blaze_message_t* p_message)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t len = ANT_BLAZE_MAX_MESSAGE_LENGTH + 2;
    uint8_t char_value[ANT_BLAZE_MAX_MESSAGE_LENGTH + 2];

    if(p_sfs == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if((p_sfs->conn_handle == BLE_CONN_HANDLE_INVALID) && (!p_sfs->is_notification_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    // Serialize mesh message. Include index and data payload. Length is fixed to ANT_BLAZE_MAX_MESSAGE_LENGTH + 2
    // and pad with zeroes.
    memset(char_value, 0, len);
    char_value[0] = p_message->index & 0xFF;
    char_value[1] = (p_message->index >> 8) & 0xFF;
    memcpy(&char_value[2], p_message->p_data, p_message->length);

    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.handle = p_sfs->sf_rx_handles.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.p_data = char_value;
    hvx_params.p_len  = &len;

    return sd_ble_gatts_hvx(p_sfs->conn_handle, &hvx_params);
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_sfs     Service structure.
 * @param[in] p_evt_write Pointer to the event received from BLE stack.
 */
static void on_write(ble_sf_service_t * p_sfs, ble_gatts_evt_write_t * p_evt_write)
{
    if((p_evt_write->handle == p_sfs->sf_tx_handles.cccd_handle) &&
       (p_evt_write->len == 2))
    {
        if(ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_sfs->is_notification_enabled = true;
        }
        else
        {
            p_sfs->is_notification_enabled = false;
        }
    }
    else if ((p_evt_write->handle == p_sfs->sf_tx_handles.value_handle) &&
             (p_sfs->evt_handler != NULL))
    {
        // Pass received data to handler, as is
        p_sfs->evt_handler(p_evt_write->data, p_evt_write->len);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}

/**@brief Function for adding ANT BLAZE characteristic.
 *
 * @param[in] p_sfs       Service structure.
 * @param[in] p_sfs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sf_message_tx_char_add(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;

    ble_uuid.type = p_sfs->uuid_type;
    ble_uuid.uuid = BLE_UUID_SFS_TX_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.max_len   = ANT_BLAZE_MAX_MESSAGE_LENGTH;
    attr_char_value.p_value   = m_tx_char_value;

    return sd_ble_gatts_characteristic_add(p_sfs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_sfs->sf_tx_handles);
}


static uint32_t sf_message_rx_char_add(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Configure Client Characteristic Configuration Descriptor metadata. No extra security needed to read/write.
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;    // CCCD stored in stack

    // Configure characteristic metadata
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.p_cccd_md                = &cccd_md;

    // Use custom UUID
    ble_uuid.type = p_sfs->uuid_type;
    ble_uuid.uuid = BLE_UUID_SFS_RX_CHARACTERISTIC;

    // Configure the attribute metadata. No extra security needed to read/write.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_USER;  // Characteristic value stored in user memory
    attr_md.vlen    = 1;

    // Configure the characteristic value attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.max_len   = ANT_BLAZE_MAX_MESSAGE_LENGTH + 2;
    attr_char_value.p_value   = m_rx_char_value;

    // Add characteristic to service
    return sd_ble_gatts_characteristic_add(p_sfs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_sfs->sf_rx_handles);
}

#endif

