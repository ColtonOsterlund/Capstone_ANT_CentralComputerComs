/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2016
All rights reserved.
*/

#if defined(BLE_STACK_SUPPORT_REQD)
#include "sdk_config.h"
#include "app_timer.h"
#include "app_error.h"
#include "ble.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_app_interface.h"
#include "nrf_log.h"

#define CONN_CFG_TAG                        1                                           /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */

#define APP_FEATURE_NOT_SUPPORTED           BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define DEVICE_NAME                         "ANTBlazeNode"                             /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL                    1636                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1022.5 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS          0                                           /**< The advertising timeout in units of seconds - Disabled. */

#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(210, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (ms). Must be at least 20ms. */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(230, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (ms). Must be at least 20ms more than min interval, and max 2 s. */
#define SLAVE_LATENCY                       0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(5000, UNIT_10_MS)             /**< Connection supervisory timeout (ms) Max 6s as per Apple's Bluetooth Design Guidelines. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define HVN_TX_QUEUE_SIZE                   20                                          /**< Size of handle value notification tx queue. */

static uint16_t m_conn_handle =     BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_sf_service_t m_sfs;
static ble_uuid_t m_adv_uuids[] =   {{BLE_UUID_SF_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}}; /**< Universally unique service identifiers. */

static ble_advdata_t m_advdata;         /**< Advertisement data struct */
static ble_advdata_t m_scanrsp;         /**< Scan response data struct */

static uint8_t                  m_adv_manuf_data_data[2] = {0};  /**< Advertising manufacturer specific data array. */

/**< Advertising manufacturer specific data structure. */
static ble_advdata_manuf_data_t m_adv_manuf_data =
{
    0xFFFF, // Manufacturer ID reserved for development.
    {
        2,
        m_adv_manuf_data_data
    }
};

static void bai_gap_params_init(void);
static void bai_conn_params_init(void);
static void bai_advertising_init(void);
static void bai_conn_params_error_handler(uint32_t nrf_error);
static void bai_on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void bai_on_ble_evt(ble_evt_t * p_ble_evt);


void bai_stack_init(void)
{
    uint32_t err_code;
    uint32_t app_ram_base;
    ble_cfg_t ble_cfg;

#if defined ( __CC_ARM )
    extern uint32_t Image$$RW_IRAM1$$Base;
    const volatile uint32_t ram_start = (uint32_t) &Image$$RW_IRAM1$$Base;
#elif defined ( __ICCARM__ )
    extern uint32_t __ICFEDIT_region_IRAM1_start__;
    volatile uint32_t ram_start = (uint32_t) &__ICFEDIT_region_IRAM1_start__;
#elif defined   ( __GNUC__ )
    extern uint32_t __data_start__;
    volatile uint32_t ram_start = (uint32_t) &__data_start__;
#endif

    // Fetch the start address of the application RAM
    app_ram_base = ram_start;

    // Overwrite some of the default configurations for the BLE stack

    // Configure number of custom UUIDS.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 1;
    err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, app_ram_base);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum number of connections
    memset(&ble_cfg, 0, sizeof(ble_cfg_t));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, app_ram_base);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum ATT MTU
    memset(&ble_cfg, 0, sizeof(ble_cfg_t));
    ble_cfg.conn_cfg.conn_cfg_tag = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_BLE_GATT_MAX_MTU_SIZE;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, app_ram_base);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum event length
    memset(&ble_cfg, 0, sizeof(ble_cfg_t));
    ble_cfg.conn_cfg.conn_cfg_tag = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_DEFAULT; // 3 * 1.25 ms = 3.75 ms
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count = BLE_GAP_CONN_COUNT_DEFAULT;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, app_ram_base);
    APP_ERROR_CHECK(err_code);

    // Configure the HVN queue size.
    // Because mesh events can take place quicker than the connection interval, we need to queue them
    memset(&ble_cfg, 0, sizeof(ble_cfg_t));
    ble_cfg.conn_cfg.conn_cfg_tag = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size = HVN_TX_QUEUE_SIZE;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &ble_cfg, app_ram_base);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack
    err_code = sd_ble_enable(&app_ram_base);
    APP_ERROR_CHECK(err_code);
}


void bai_init(ble_sf_service_evt_handler_t evt_handler)
{
    uint32_t       err_code;
    ble_sf_service_init_t ams_init;

    bai_gap_params_init();

    memset(&ams_init, 0, sizeof(ams_init));
    ams_init.evt_handler = evt_handler;

    err_code = ble_sf_service_init(&m_sfs, &ams_init);
    APP_ERROR_CHECK(err_code);

    bai_advertising_init();
    bai_conn_params_init();
}


void bai_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_sf_service_on_ble_evt(&m_sfs, p_ble_evt);
    bai_on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}


void bai_advertising_start(uint16_t node_id)
{
    uint32_t err_code;

    // Add the node ID in the manufacturer specific adv data
    m_adv_manuf_data.data.p_data[0] = node_id & 0xFF;
    m_adv_manuf_data.data.p_data[1] = (node_id >> 8) & 0xFF;
    err_code = ble_advdata_set(&m_advdata, &m_scanrsp);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


void bai_update_rx_char(ant_blaze_message_t* p_msg)
{
    uint32_t err_code = ble_sf_service_update_rx_char(&m_sfs, p_msg);
    // Invalid state is received if we try to update before we are connected or
    // while notifications are not enabled. This error just indicates
    // that the characteristic was not updated in these cases
    // so we can ignore the error.
    if(err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
}


static void bai_gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void bai_conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = bai_conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


static void bai_advertising_init(void)
{
    uint32_t      err_code;
    ble_adv_modes_config_t options;

    // Build and set advertising data
    memset(&m_advdata, 0, sizeof(m_advdata));
    m_advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    m_advdata.include_appearance = false;
    m_advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE; // stay advertising forever
    m_advdata.p_manuf_specific_data = &m_adv_manuf_data;

    memset(&m_scanrsp, 0, sizeof(m_scanrsp));
    m_scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    m_scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&m_advdata, &m_scanrsp, &options, bai_on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(CONN_CFG_TAG);
}


static void bai_conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


static void bai_on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    NRF_LOG_INFO("BLE advertising evt: 0x%02x\r\n", ble_adv_evt);
}



 static void bai_on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    NRF_LOG_INFO("BLE evt: 0x%02x\r\n", p_ble_evt->header.evt_id);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("BLE connected. Negotiated connection parameters: %d, %d, %d\r\n",
                            p_ble_evt->evt.gap_evt.params.connected.conn_params.min_conn_interval,
                            p_ble_evt->evt.gap_evt.params.connected.conn_params.max_conn_interval,
                            p_ble_evt->evt.gap_evt.params.connected.conn_params.conn_sup_timeout);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            // Because we don't use any bonding, there will never be any system attributes stored.
            // Set them now, so that there are no BLE_ERROR_GATSS_SYS_ATTR_MISSING when attempting
            // to update the characteristic before processing the BLE_GATTS_EVT_SYS_ATTR_MISSING
            // event.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE disconnected. Reason: %d\r\n",
                            p_ble_evt->evt.gap_evt.params.disconnected.reason);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            NRF_LOG_INFO("BLE connection parameter update: %d, %d, %d\r\n",
                p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.min_conn_interval,
                p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval,
                p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.conn_sup_timeout);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored. Should not see this error since
            // the system attributes are set on the connection event.
            NRF_LOG_DEBUG("GATT System Attributes Missing.\r\n");
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            NRF_LOG_INFO("GATTS Exchange MTU Request \r\n");
#if (NRF_SD_BLE_API_VERSION >= 3)
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle, NRF_BLE_GATT_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
#endif
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST

        default:
            // No implementation needed.
            break;
    }
}

#endif

