/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2016
All rights reserved.
*/

#ifndef __BLE_SETUP_H
#define __BLE_SETUP_H

#include <stdint.h>
#include "ble_sf_service.h"
#include "ble_hci.h"

/**@brief BLE stack initialization.
 * Note that this initialization code does not make use of the
 * softdevice_handler, and it does not configure an event
 * handler for BLE events; instead, the application must pass
 * to this module the events through bai_evt_dispatch.
 *
 * @details Initializes the BLE stack
 */
void bai_stack_init(void);


/**@brief BLE initialization
 * This function sets up all the necessary GAP parameters of the
 * device, connection parameters, advertising, as well as initializes
 * the custom service
 * @param   evt_handler     Event handler for custom service events
 */
void bai_init(ble_sf_service_evt_handler_t evt_handler);


/**@brief Process BLE events
 *
 * @details This function is called after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void bai_evt_dispatch(ble_evt_t * p_ble_evt);


/**@brief Function for starting advertising
*/
void bai_advertising_start(uint16_t node_id);


/**@brief Update the value of Rx characteristic with message
 * received over the mesh network
 * @param[in] p_msg     Pointer to message struct*/
void bai_update_rx_char(ant_blaze_message_t* p_msg);



#endif
