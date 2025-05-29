#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <lvgl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

LOG_MODULE_REGISTER(app);

#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240
#define DOT_SIZE         10

#define LOGICAL_WIDTH_M  3.6f
#define LOGICAL_HEIGHT_M 3.0f

#define VIEW_X_MIN 0.0f
#define VIEW_X_MAX 4.0f
#define VIEW_Y_MIN -0.5f
#define VIEW_Y_MAX 3.0f

#define UUID_COORD_SERVICE \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x91fc7973, 0x6500, 0x4385, 0x9d19, 0xdc705a3b3de9))
#define UUID_COORD_CHAR \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x91fc7974, 0x6500, 0x4385, 0x9d19, 0xdc705a3b3de9))

static lv_obj_t *coord_label;

struct pos {
    float x, y;
};

static void viewport_transform(float x_m, float y_m, int *px, int *py) {
    float norm_x = (x_m - VIEW_X_MIN) / (VIEW_X_MAX - VIEW_X_MIN);
    float norm_y = (y_m - VIEW_Y_MIN) / (VIEW_Y_MAX - VIEW_Y_MIN);
    *px = (int)(norm_x * SCREEN_WIDTH);
    *py = (int)((1.0f - norm_y) * SCREEN_HEIGHT);
}

static void update_position(float x, float y) {
    LV_UNUSED(x);
    LV_UNUSED(y);

    float humidity = 55.3f;
    float temperature = 24.7f;
    int rssi_avg = -67;

    lv_label_set_text_fmt(coord_label,
        "Humidity: %.1f %%\n"
        "Temperature: %.1f °C\n"
        "RSSI Avg: %d dBm",
        humidity, temperature, rssi_avg);
}

static ssize_t write_coords(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                            const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
    if (len >= 14) {
        const uint8_t *data = buf;

        uint16_t company_id     = (data[1] << 8) | data[0];
        uint16_t co2_value      = (data[3] << 8) | data[2];
        uint16_t humid_value    = (data[5] << 8) | data[4];
        uint16_t temp_value     = (data[7] << 8) | data[6];
        uint16_t angle_value    = (data[9] << 8) | data[8];
        uint32_t distance_value = (data[13] << 24) | (data[12] << 16) | (data[11] << 8) | data[10];

        LOG_INF("Company ID: %u", company_id);
        LOG_INF("CO2: %u ppm", co2_value);
        LOG_INF("Humidity: %.2f %%", humid_value / 100.0f);
        LOG_INF("Temperature: %.2f °C", temp_value / 100.0f);
        LOG_INF("Angle: %u deg", angle_value);
        LOG_INF("Distance: %.2f cm", distance_value / 100.0f);

        lv_label_set_text_fmt(coord_label,
            "CO2: %u ppm\n"
            "Humidity: %u.%02u %%\n"
            "Temp: %u.%02u °C\n"
            "Angle: %u°\n"
            "Dist: %u.%02u cm",
            co2_value,
            humid_value / 100, humid_value % 100,
            temp_value / 100, temp_value % 100,
            angle_value,
            distance_value / 100, distance_value % 100);
    }

    return len;
}

BT_GATT_SERVICE_DEFINE(coord_svc,
    BT_GATT_PRIMARY_SERVICE(UUID_COORD_SERVICE),
    BT_GATT_CHARACTERISTIC(UUID_COORD_CHAR,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, write_coords, NULL),
);

int main(void) {
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display not ready");
        return 0;
    }

    lv_init();
    lv_obj_t *bg = lv_scr_act();  // Use root screen directly
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xFFFFFF), 0);  // White background

    coord_label = lv_label_create(bg);
    lv_obj_align(coord_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(coord_label, "Waiting for data...");

    display_blanking_off(display_dev);
    lv_task_handler();

    int err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return 1;
    }
    LOG_INF("Bluetooth initialized");

    static struct bt_le_adv_param adv_param = {
        .id = BT_ID_DEFAULT,
        .sid = 0,
        .secondary_max_skip = 0,
        .options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
        .peer = NULL,
    };

    err = bt_le_adv_start(&adv_param, NULL, 0, NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return 1;
    }
    LOG_INF("Advertising started");

    while (1) {
        lv_task_handler();
        k_sleep(K_MSEC(100));
    }
}