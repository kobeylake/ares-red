#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/timing/timing.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/byteorder.h>
#include <string.h>

#define COMPANY_ID_LSB     0xAF
#define COMPANY_ID_MSB     0x6F
#define APP_ID_MOBILE      0xBB

#define TRIG_PIN 10
#define ECHO_PIN 9
#define TIMEOUT_US 25000
#define GPIO1_NODE DT_NODELABEL(gpio1)

// Format: [Company ID (0xFFFF)], [App ID (0xAA)], [Desired angle - to send to the other nrf board (in degrees)]
static uint8_t mfg_data[4] = {
    0xAF, 0x6F, // Fake company ID (0xFFFF) — for prototyping
    0xAA,       // Custom App ID
    0x00  // Placeholder for servo angle (0-90deg)
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
    
    // bt_addr_le_t addr;
    // size_t count = 1;
    // bt_id_get(&addr, &count);
    // char addr_str[BT_ADDR_LE_STR_LEN];
    // bt_addr_le_to_str(&addr, addr_str, sizeof(addr_str));
    // printk("Bluetooth MAC address: %s\n", addr_str);
    

    int err = bt_le_adv_start(adv_params, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("BLE advertising failed to start (err %d)\n", err);
        return 1;
    } else {
        printk("BLE advertising started successfully\n");
    }

    gpio_pin_configure(gpio1, TRIG_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(gpio1, ECHO_PIN, GPIO_INPUT);

    while (1) {
        // Send pulse
        gpio_pin_set(gpio1, TRIG_PIN, 1);
        k_busy_wait(10);
        gpio_pin_set(gpio1, TRIG_PIN, 0);

        // Wait for rising edge
        uint32_t timeout = TIMEOUT_US;
        while (gpio_pin_get(gpio1, ECHO_PIN) == 0 && timeout--) {
            k_busy_wait(1);
        }
        if (timeout == 0) {
            printk("No echo rising edge\n");
            continue;
        }

        uint32_t start = k_cycle_get_32();

        // Wait for falling edge
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

        // use a kalman filter to calcualte optimal opening angle, for the servo 
        // motor on another nrf board
        mfg_data[3] = 69; // calcualted angle to send over ble

        // Update advertising payload
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

        k_sleep(K_MSEC(500));
    }
}
