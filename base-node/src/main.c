#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <string.h>

#define COMPANY_ID         0x6FAF  // LSB: 0xAF, 0x6F
#define APP_ID_MOBILE      0xBB
#define APP_ID_BASE        0xAA

#define TRIG_PIN 10
#define ECHO_PIN 9
#define TIMEOUT_US 25000
#define GPIO1_NODE DT_NODELABEL(gpio1)

// BLE Advertising payload (used to send servo angle)
static uint8_t mfg_data[4] = {
    0xAF, 0x6F, // Company ID
    APP_ID_BASE,
    0x00        // Placeholder for servo angle (0-90)
};

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
};

static const struct bt_le_adv_param *adv_params = BT_LE_ADV_PARAM(
    BT_LE_ADV_OPT_USE_NAME,
    BT_GAP_ADV_FAST_INT_MIN_2,
    BT_GAP_ADV_FAST_INT_MAX_2,
    NULL
);

// Scanner callback
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad) {
    while (ad->len > 1) {
        uint8_t len = net_buf_simple_pull_u8(ad);
        if (len == 0 || len > ad->len) break;

        uint8_t field_type = net_buf_simple_pull_u8(ad);
        len--;

        if (field_type == BT_DATA_MANUFACTURER_DATA && len >= 6) {
            uint16_t company_id = sys_get_le16(ad->data);

            if (company_id == COMPANY_ID && ad->data[2] == APP_ID_MOBILE) {
                const uint8_t *payload = ad->data + 3;

                int16_t temp_raw = (payload[0] << 8) | payload[1];
                uint16_t co2 = (payload[2] << 8) | payload[3];
                uint16_t humid_raw = (payload[4] << 8) | payload[5];

                float temp = temp_raw / 100.0f;
                float humid = humid_raw / 100.0f;

                printk("Received Mobile Node Data:\n");
                printk("  Temp: %.2f°C\n", temp);
                printk("  CO2: %u ppm\n", co2);
                printk("  Humidity: %.2f%%\n", humid);
            }

            net_buf_simple_pull(ad, len);
        } else {
            net_buf_simple_pull(ad, len);
        }
    }
}

// Scanner thread
void scanner_thread_fn(void) {
    struct bt_le_scan_param scan_params = {
        .type     = BT_LE_SCAN_TYPE_PASSIVE,
        .options  = BT_LE_SCAN_OPT_NONE,
        .interval = 0x0010,
        .window   = 0x0010,
    };

    int err = bt_le_scan_start(&scan_params, device_found);
    if (err) {
        printk("BLE scan start failed (err %d)\n", err);
        return;
    }

    printk("BLE scanner started\n");

    while (1) {
        k_sleep(K_SECONDS(1));
    }
}

// Declare scanner thread
#define SCANNER_STACK_SIZE 1024
#define SCANNER_PRIORITY 4
K_THREAD_STACK_DEFINE(scanner_stack, SCANNER_STACK_SIZE);
static struct k_thread scanner_thread_data;

int main(void)
{
    const struct device *gpio1 = DEVICE_DT_GET(GPIO1_NODE);
    if (!device_is_ready(gpio1)) {
        printk("GPIO1 not ready\n");
        return 1;
    }

    if (bt_enable(NULL)) {
        printk("Bluetooth init failed\n");
        return 1;
    }

    int err = bt_le_adv_start(adv_params, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("BLE advertising failed to start (err %d)\n", err);
        return 1;
    }

    printk("BLE advertising started\n");

    // Start scanner thread
    k_thread_create(&scanner_thread_data, scanner_stack, SCANNER_STACK_SIZE,
                    (k_thread_entry_t)scanner_thread_fn, NULL, NULL, NULL,
                    SCANNER_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&scanner_thread_data, "ble_scanner");

    gpio_pin_configure(gpio1, TRIG_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(gpio1, ECHO_PIN, GPIO_INPUT);

    while (1) {
        // Trigger ultrasonic measurement
        gpio_pin_set(gpio1, TRIG_PIN, 1);
        k_busy_wait(10);
        gpio_pin_set(gpio1, TRIG_PIN, 0);

        uint32_t timeout = TIMEOUT_US;
        while (gpio_pin_get(gpio1, ECHO_PIN) == 0 && timeout--) {
            k_busy_wait(1);
        }
        if (timeout == 0) {
            printk("No echo rising edge\n");
            continue;
        }

        uint32_t start = k_cycle_get_32();

        timeout = TIMEOUT_US;
        while (gpio_pin_get(gpio1, ECHO_PIN) == 1 && timeout--) {
            k_busy_wait(1);
        }
        if (timeout == 0) {
            printk("No echo falling edge\n");
            continue;
        }

        uint32_t end = k_cycle_get_32();
        uint64_t time_ns = k_cyc_to_ns_floor64(end - start);
        float time_us = time_ns / 1000.0f;
        float distance_cm = time_us / 58.0f;
        float distance_mm = distance_cm * 10.0f;

        if (distance_mm > 65535) distance_mm = 65535;
        uint16_t dist_encoded = (uint16_t)distance_mm;

        printk("Distance: encoded as %u mm\n", dist_encoded);

        // Simulate calculation of servo angle (e.g., using Kalman filter)
        mfg_data[3] = 69; // placeholder for calculated angle

        // Update BLE advertisement
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

        k_sleep(K_MSEC(500));
    }
}
