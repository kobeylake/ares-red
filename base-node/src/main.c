#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <string.h>
#include <zephyr/data/json.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>

#define COMPANY_ID         0x6FAF  // LSB: 0xAF, 0x6F
#define APP_ID_MOBILE      0xBB
#define APP_ID_BASE        0xAA

#define TRIG_PIN 10
#define ECHO_PIN 9
#define TIMEOUT_US 25000
#define GPIO1_NODE DT_NODELABEL(gpio1)

#define DIST_RING_SIZE 16

static uint16_t distance_ring[DIST_RING_SIZE];
static size_t distance_ring_head = 0;
K_MUTEX_DEFINE(distance_mutex);


struct sensor_json_msg {
    int32_t company_id;
    int32_t co2_value;
    int32_t humid_value;
    int32_t temp_value;
    int32_t angle_value;
    int32_t distance_value;
};

static const struct json_obj_descr sensor_json_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, company_id, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, co2_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, humid_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, temp_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, angle_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor_json_msg, distance_value, JSON_TOK_NUMBER)
};

// BLE Advertising payload (used to send servo angle)
static uint8_t mfg_data[12] = {
    0xAF, 0x6F, // Company ID
    APP_ID_BASE,
    0x00, 0x00, // co2
    0x00, 0x00, //humidity
    0x00, 0x00, //temp
    0x00, // Placeholder for servo angle (0-90)
    0x00, 0x00// distance
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

static bool ultrasonic_enabled = true;
static bool manual_mode = false;
static uint8_t manual_angle = 0;
static bool sensor_override = false;
static int overridden_co2 = 400;
static int overridden_temp = 25;

uint8_t calculate_angle(int co2_ppm, int temp_c){

    float angle = (co2_ppm / 1000.0f) * 90.0f; // Scale CO2 to 0-90 degrees
    angle += (temp_c - 20) * 1.5f; // Adjust based on temperature

    if (angle < 0) angle = 0;
    if (angle > 90) angle = 90;

    return (uint8_t)angle;
}

// Scanner callback
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *buf) {
    while (buf->len > 1) {
        uint8_t len = net_buf_simple_pull_u8(buf);
        if (len == 0 || len > buf->len) break;

        uint8_t field_type = net_buf_simple_pull_u8(buf);
        len--;

        if (field_type == BT_DATA_MANUFACTURER_DATA && len >= 6) {
            uint16_t company_id = sys_get_le16(buf->data);

            if (company_id == COMPANY_ID && buf->data[2] == APP_ID_MOBILE) {
                const uint8_t *payload = buf->data + 3;

                int16_t temp_raw = (payload[0] << 8) | payload[1];
                uint16_t co2_raw = (payload[2] << 8) | payload[3];
                uint16_t humid_raw = (payload[4] << 8) | payload[5];

                uint16_t temp_int = temp_raw / 100;
                uint16_t temp_frac = temp_raw % 100;

                uint16_t humid_int = humid_raw / 100;
                uint16_t humid_frac = humid_raw % 100;

                // Write 16-bit CO2
                sys_put_le16(co2_raw, &mfg_data[3]);

                // Write 16-bit humidity
                sys_put_le16(humid_raw, &mfg_data[5]);

                // Write 16-bit temperature
                sys_put_le16(temp_raw, &mfg_data[7]);

                uint16_t latest_distance = 0;
                k_mutex_lock(&distance_mutex, K_FOREVER);
                size_t last_idx = (distance_ring_head + DIST_RING_SIZE - 1) % DIST_RING_SIZE;
                latest_distance = distance_ring[last_idx];
                k_mutex_unlock(&distance_mutex);

                sys_put_le16(latest_distance, &mfg_data[10]);

                // printk("Received Mobile Node Data:\n");
                // printk("  Temp: %d.%02d°C\n", temp_int, temp_frac);
                // printk("  CO2: %u ppm\n", co2_raw);
                // printk("  Humidity: %d.%02d%%\n", humid_int, humid_frac);

                int used_co2 = sensor_override ? overridden_co2 : co2_raw;
                int used_temp = sensor_override ? overridden_temp : temp_int;
                uint8_t angle = manual_mode ? manual_angle : calculate_angle(used_co2, used_temp);
                mfg_data[9] = angle;
                bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

                struct sensor_json_msg msg = {
                    .company_id = COMPANY_ID,
                    .co2_value = co2_raw,
                    .humid_value = humid_raw,
                    .temp_value = temp_raw,
                    .angle_value = angle,
                    .distance_value = latest_distance
                };              

                char buffer[256];
                int ret = json_obj_encode_buf(sensor_json_descr, ARRAY_SIZE(sensor_json_descr),
                                              &msg, buffer, sizeof(buffer));
                if (ret == 0) {
                    printk("%s\n", buffer);
                } else {
                    printk("JSON encode failed (err %d)\n", ret);
                }
            }

            net_buf_simple_pull(buf, len);
        } else {
            net_buf_simple_pull(buf, len);
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

void ultrasonic_thread_fn(void *gpio_dev, void *unused1, void *unused2) {
    const struct device *gpio1 = gpio_dev;

    while (1) {
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

        k_mutex_lock(&distance_mutex, K_FOREVER);
        distance_ring[distance_ring_head] = dist_encoded;
        distance_ring_head = (distance_ring_head + 1) % DIST_RING_SIZE;
        k_mutex_unlock(&distance_mutex);

        //printk("Distance: encoded as %u mm\n", dist_encoded);
        k_sleep(K_MSEC(500));
    }
}

// Declare scanner thread
#define SCANNER_STACK_SIZE 1024
#define SCANNER_PRIORITY 4
K_THREAD_STACK_DEFINE(scanner_stack, SCANNER_STACK_SIZE);
static struct k_thread scanner_thread_data;

// Declare ultrasonic thread
#define ULTRASONIC_STACK_SIZE 1024
#define ULTRASONIC_PRIORITY   4
K_THREAD_STACK_DEFINE(ultrasonic_stack, ULTRASONIC_STACK_SIZE);
static struct k_thread ultrasonic_thread_data;

static int cmd_ultrasonic_on(const struct shell *shell, size_t argc, char **argv) {
    if (!ultrasonic_enabled) {
        k_thread_resume(&ultrasonic_thread_data);
        ultrasonic_enabled = true;
        shell_print(shell, "Ultrasonic sensor enabled.");
    } else {
        shell_print(shell, "Ultrasonic sensor already enabled.");
    }
    return 0;
}

static int cmd_ultrasonic_off(const struct shell *shell, size_t argc, char **argv) {
    if (ultrasonic_enabled) {
        k_thread_suspend(&ultrasonic_thread_data);
        ultrasonic_enabled = false;
        shell_print(shell, "Ultrasonic sensor disabled.");
    } else {
        shell_print(shell, "Ultrasonic sensor already disabled.");
    }
    return 0;
}

static int cmd_servo_set(const struct shell *shell, size_t argc, char **argv) {
    int angle = atoi(argv[1]);
    if (angle < 0 || angle > 90) {
        shell_print(shell, "Angle must be between 0 and 90.");
        return -EINVAL;
    }
    manual_angle = angle;
    mfg_data[3] = manual_angle;
    manual_mode = true;
    shell_print(shell, "Servo angle set to %d (manual mode).", angle);
    return 0;
}

static int cmd_servo_mode(const struct shell *shell, size_t argc, char **argv) {
    if (strcmp(argv[1], "manual") == 0) {
        manual_mode = true;
        mfg_data[3] = manual_angle;
        shell_print(shell, "Switched to manual servo control.");
    } else if (strcmp(argv[1], "auto") == 0) {
        manual_mode = false;
        shell_print(shell, "Switched to automatic servo control.");
    } else {
        shell_print(shell, "Usage: servo_mode [manual|auto]");
        return -EINVAL;
    }
    return 0;
}

static int cmd_servo_get(const struct shell *shell, size_t argc, char **argv) {
    shell_print(shell, "Current advertised servo angle: %d (mode: %s)",
                mfg_data[3], manual_mode ? "manual" : "auto");
    return 0;
}

static int cmd_sensor_override(const struct shell *shell, size_t argc, char **argv) {
    overridden_co2 = atoi(argv[1]);
    overridden_temp = atoi(argv[2]);
    sensor_override = true;
    shell_print(shell, "Sensor override enabled: CO2=%d ppm, Temp=%d C", overridden_co2, overridden_temp);
    return 0;
}

static int cmd_sensor_use_real(const struct shell *shell, size_t argc, char **argv) {
    sensor_override = false;
    shell_print(shell, "Sensor override disabled. Using real data.");
    return 0;
}

static int cmd_sensor_status(const struct shell *shell, size_t argc, char **argv) {
    if (sensor_override) {
        shell_print(shell, "Override ACTIVE - CO2: %d ppm, Temp: %d C", overridden_co2, overridden_temp);
    } else {
        shell_print(shell, "Override DISABLED - using real sensor data.");
    }
    return 0;
}

SHELL_CMD_REGISTER(ultrasonic_on, NULL, "Enable ultrasonic sensor", cmd_ultrasonic_on);
SHELL_CMD_REGISTER(ultrasonic_off, NULL, "Disable ultrasonic sensor", cmd_ultrasonic_off);
SHELL_CMD_REGISTER(servo_set, NULL, "Set servo angle (0-90)", cmd_servo_set);
SHELL_CMD_REGISTER(servo_mode, NULL, "Set servo mode [manual|auto]", cmd_servo_mode);
SHELL_CMD_REGISTER(servo_get, NULL, "Get current servo angle", cmd_servo_get);
SHELL_CMD_REGISTER(sensor_override, NULL, "Override sensor CO2 and Temp", cmd_sensor_override);
SHELL_CMD_REGISTER(sensor_use_real, NULL, "Use real sensor data", cmd_sensor_use_real);
SHELL_CMD_REGISTER(sensor_status, NULL, "Show sensor override status", cmd_sensor_status);

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

    // Start ultrasonic thread
    k_thread_create(&ultrasonic_thread_data, ultrasonic_stack, ULTRASONIC_STACK_SIZE,
                    ultrasonic_thread_fn, (void *)gpio1, NULL, NULL,
                    ULTRASONIC_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&ultrasonic_thread_data, "ultrasonic_thread");

    while (1) {
        k_sleep(K_FOREVER);
    }
}
