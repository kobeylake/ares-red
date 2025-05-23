#include "bluetooth.h"
#include "sensor.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble_module, LOG_LEVEL_INF);

// 6 bytes: 2 for temp, 2 for CO2, 2 for humidity
static uint8_t adv_payload[6];

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_payload, sizeof(adv_payload)),
};

// BLE advertiser thread
#define BLE_STACK_SIZE 1024
#define BLE_PRIORITY   5
K_THREAD_STACK_DEFINE(ble_stack, BLE_STACK_SIZE);
static struct k_thread ble_thread_data;

static void bluetooth_thread_fn(void *a, void *b, void *c) {
    struct greenhouse_data snapshot;

    while (1) {
        // Copy shared sensor data
        k_mutex_lock(&ghs_mutex, K_FOREVER);
        snapshot = ghs_data;
        k_mutex_unlock(&ghs_mutex);

        // Encode values
        int16_t temp = snapshot.temperature.val1 * 100 + snapshot.temperature.val2 / 10000;
        uint16_t co2 = snapshot.co2.val1;
        uint16_t humid = snapshot.humidity.val1 * 100 + snapshot.humidity.val2 / 10000;

        adv_payload[0] = temp >> 8;
        adv_payload[1] = temp & 0xFF;
        adv_payload[2] = co2 >> 8;
        adv_payload[3] = co2 & 0xFF;
        adv_payload[4] = humid >> 8;
        adv_payload[5] = humid & 0xFF;

        // Update BLE advertisement
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

        // should update to be JSON format
        LOG_INF("BLE Update: Temp=%d.%02d°C, CO₂=%dppm, Humidity=%d.%02d%%",
                snapshot.temperature.val1, snapshot.temperature.val2 / 10000,
                co2,
                snapshot.humidity.val1, snapshot.humidity.val2 / 10000);

        k_sleep(K_SECONDS(5));
    }
}

void bluetooth_init(void) {
    int err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    const struct bt_le_adv_param *adv_params = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_USE_NAME,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL
    );

    err = bt_le_adv_start(adv_params, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("BLE advertising start failed (err %d)", err);
        return;
    }

    LOG_INF("BLE advertising started");
}

void bluetooth_task_start(void) {
    k_thread_create(&ble_thread_data, ble_stack, BLE_STACK_SIZE,
                    bluetooth_thread_fn, NULL, NULL, NULL,
                    BLE_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&ble_thread_data, "ble_adv_thread");
}
