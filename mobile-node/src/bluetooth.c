#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>
#include "sensor.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

static uint8_t manuf_data[4];

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, manuf_data, sizeof(manuf_data)),
};

void bluetooth_update_payload(struct greenhouse_data *data) {
    int16_t temp = data->temperature.val1 * 100 + data->temperature.val2 / 10000;
    uint16_t co2 = data->co2.val1;

    manuf_data[0] = temp >> 8;
    manuf_data[1] = temp & 0xFF;
    manuf_data[2] = co2 >> 8;
    manuf_data[3] = co2 & 0xFF;

    bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
    LOG_INF("Updated BLE advertisement: temp=%d.%02d°C, CO₂=%dppm", data->temperature.val1, data->temperature.val2 / 10000, co2);
}

void bluetooth_init(void) {
    int err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed");
        return;
    }

    LOG_INF("Bluetooth enabled");

    struct bt_le_adv_param *adv_params =
        BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_NAME,
                        BT_GAP_ADV_FAST_INT_MIN_2,
                        BT_GAP_ADV_FAST_INT_MAX_2,
                        NULL);

    bt_le_adv_start(adv_params, ad, ARRAY_SIZE(ad), NULL, 0);
    LOG_INF("BLE advertising started");
}
