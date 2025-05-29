#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/byteorder.h>

#define PWM_THREAD_STACK_SIZE 1024
#define PWM_THREAD_PRIORITY 5

#define SERVO_PERIOD PWM_MSEC(20)          // 20 ms = 50 Hz
#define SERVO_MIN_PULSE PWM_USEC(1000)     // 1 ms pulse (~0 degrees)
#define SERVO_MAX_PULSE PWM_USEC(2000)     // 2 ms pulse (~90 degrees)

#define PWM_NODE DT_ALIAS(pwmsignal)
static const struct pwm_dt_spec pwm_signal = PWM_DT_SPEC_GET(PWM_NODE);

#define COMPANY_ID 0x6FAF
#define APP_ID_BASE 0xAA

static uint8_t current_angle = 0;
static struct k_mutex angle_mutex;

void update_servo(uint8_t angle)
{
    uint32_t pulse = SERVO_MIN_PULSE +
                     ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle) / 90;

    int ret = pwm_set_dt(&pwm_signal, SERVO_PERIOD, pulse);
    if (ret < 0) {
        printk("PWM set failed (%d)\n", ret);
    } else {
        printk("Set servo to angle %d (%u us pulse)\n", angle, pulse);
    }
}

// BLE device scan callback
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad)
{
    while (ad->len > 1) {
        uint8_t len = net_buf_simple_pull_u8(ad);
        if (len == 0 || len > ad->len) break;

        uint8_t field_type = net_buf_simple_pull_u8(ad);
        len--;

        if (field_type == BT_DATA_MANUFACTURER_DATA && len >= 4) {
            uint16_t company_id = sys_get_le16(ad->data);
            uint8_t app_id = ad->data[2];
            uint8_t angle = ad->data[3];

            if (company_id == COMPANY_ID && app_id == APP_ID_BASE) {
                k_mutex_lock(&angle_mutex, K_FOREVER);
                current_angle = angle;
                k_mutex_unlock(&angle_mutex);

                printk("Received angle %d from base node\n", angle);
            }

            net_buf_simple_pull(ad, len);
        } else {
            net_buf_simple_pull(ad, len);
        }
    }
}

void ble_thread(void *a, void *b, void *c)
{
    struct bt_le_scan_param scan_params = {
        .type     = BT_LE_SCAN_TYPE_PASSIVE,
        .options  = BT_LE_SCAN_OPT_NONE,
        .interval = 0x0010,
        .window   = 0x0010,
    };

    if (bt_enable(NULL)) {
        printk("Bluetooth init failed\n");
        return;
    }

    if (bt_le_scan_start(&scan_params, device_found)) {
        printk("Scan start failed\n");
        return;
    }

    printk("BLE scanner started\n");

    while (1) {
        k_sleep(K_SECONDS(1));
    }
}

void pwm_thread(void *a, void *b, void *c)
{
    if (!device_is_ready(pwm_signal.dev)) {
        printk("PWM device %s not ready\n", pwm_signal.dev->name);
        return;
    }

    while (1) {
        k_mutex_lock(&angle_mutex, K_FOREVER);
        uint8_t angle = current_angle;
        k_mutex_unlock(&angle_mutex);

        update_servo(angle);
        k_msleep(500);
    }
}

int main(void)
{
    if (!pwm_is_ready_dt(&pwm_signal)) {
        printk("Error: PWM device %s is not ready\n", pwm_signal.dev->name);
        return 1;
    }

    k_mutex_init(&angle_mutex);

    return 0;
}

K_THREAD_DEFINE(pwm_thread_id, PWM_THREAD_STACK_SIZE, pwm_thread, NULL, NULL, NULL, PWM_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(ble_thread_id, 1024, ble_thread, NULL, NULL, NULL, 4, 0, 0);
