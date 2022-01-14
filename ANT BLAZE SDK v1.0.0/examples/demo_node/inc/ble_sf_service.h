/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2015
All rights reserved.
*/

#ifndef __BLE_SF_SERVICE_H__
#define __BLE_SF_SERVICE_H__

#if defined(BLE_STACK_SUPPORT_REQD)
#include "ble.h"
#include "ble_srv_common.h"
#include "ant_blaze_defines.h"

#define BLE_UUID_SF_SERVICE 0x0001                      /**< The UUID of the Service. */

/* Forward declaration of the ble_sf_service_t type. */
typedef struct ble_sf_service_s ble_sf_service_t;

/**@brief Service event handler type. */
typedef void (*ble_sf_service_evt_handler_t) (uint8_t* p_message, uint32_t length);

/**@brief Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_am_service_init
 *          function.
 */
typedef struct
{
    ble_sf_service_evt_handler_t evt_handler; /**< Event handler to be called */
} ble_sf_service_init_t;

/**@brief ANT Scan and Forward communication service structure */
typedef struct ble_sf_service_s
{
    uint8_t                  uuid_type;          /**< UUID type for the ANT Scan and Forward Service Base UUID. */
    uint16_t                 service_handle;     /**< Handle of ANT Scan and Forward Service (as provided by the SoftDevice). */
    ble_sf_service_evt_handler_t evt_handler;    /**< Event handler to be called for handling events in this service. */
    ble_gatts_char_handles_t sf_tx_handles;      /**< Handles related to the ANT Scan and Forward Tx characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t sf_rx_handles;      /**< Handles related to the ANT Scan and Forward Rx characteristic (as provided by the SoftDevice). */
    uint16_t                 conn_handle; /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                     is_notification_enabled;   /**< Variable to indicate if the peer has enabled notification of the RX characteristic . */
} ble_sf_service_t;

/*
 *
 * @param[out] p_sfs      ANT Scan and Forward Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_sfs_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_nus or p_nus_init is NULL.
 */
uint32_t ble_sf_service_init(ble_sf_service_t * p_sfs, const ble_sf_service_init_t * p_sfs_init);


/**@brief Function for handling the ANT Scan and Forward Service's BLE events.
 *
 * @details The ANT Scan and Forward Service expects the application to call this function each time an
 * event is received from the SoftDevice. This function processes the event if it
 * is relevant and calls the ANT Scan and Forward Service event handler of the
 * application if necessary.
 *
 * @param[in] p_sfs       ANT Scan and Forward Service structure.
 * @param[in] p_ble_evt   Event received from the SoftDevice.
 */
void ble_sf_service_on_ble_evt(ble_sf_service_t* p_sfs, ble_evt_t * p_ble_evt);


/**@brief Function for sending a message received over the mesh as an Rx characteristic notification
 *
 * @param[in] p_sfs       ANT Scan and Forward Service structure.
 * @param[in] p_msg       Pointer to message to send
 */
uint32_t ble_sf_service_update_rx_char(ble_sf_service_t* p_sfs, ant_blaze_message_t* p_msg);

#endif
#endif
