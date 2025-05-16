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

static lv_obj_t *dot;
static lv_obj_t *coord_label;

struct pos {
    float x, y;
};

static struct pos beacons[8] = {
    {0.0f, 3.0f}, {1.8f, 3.0f}, {3.6f, 3.0f},
    {0.0f, 1.5f}, {3.6f, 1.5f},
    {0.0f, 0.0f}, {1.8f, 0.0f}, {3.6f, 0.0f},
};

static void viewport_transform(float x_m, float y_m, int *px, int *py) {
    float norm_x = (x_m - VIEW_X_MIN) / (VIEW_X_MAX - VIEW_X_MIN);
    float norm_y = (y_m - VIEW_Y_MIN) / (VIEW_Y_MAX - VIEW_Y_MIN);
    *px = (int)(norm_x * SCREEN_WIDTH);
    *py = (int)((1.0f - norm_y) * SCREEN_HEIGHT);
}

static void update_position(float x, float y) {
    // Use original x, y for screen positioning
    int px, py;
    viewport_transform(x, y, &px, &py);
    lv_obj_set_pos(dot, px - DOT_SIZE / 2, py - DOT_SIZE / 2);

    // Scale only for label display
    float x_disp = x / 0.9f;
    int xi = (int)(x_disp * 100);
    int yi = (int)(y * 100);
    int x_whole = xi / 100, x_frac = abs(xi % 100);
    int y_whole = yi / 100, y_frac = abs(yi % 100);

    lv_label_set_text_fmt(coord_label, "(%s%d.%02d, %s%d.%02d)",
        (x_disp < 0 ? "-" : ""), abs(x_whole), x_frac,
        (y < 0 ? "-" : ""), abs(y_whole), y_frac);
}

static ssize_t write_coords(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                            const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
    if (len >= sizeof(float) * 2) {
        float *f = (float *)buf;
        float x = f[0];
        float y = f[1];
        LOG_INF("Received coords: x=%.2lf, y=%.2lf", (double)x, (double)y);
        update_position(x , y); // <---
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
    lv_obj_t *bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(bg, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x0033aa), 0);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    dot = lv_obj_create(bg);
    lv_obj_set_size(dot, DOT_SIZE, DOT_SIZE);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0xFF0000), 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);

    coord_label = lv_label_create(bg);
    lv_obj_align(coord_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_label_set_text(coord_label, "(0.00, 0.00)");

    for (int i = 0; i < 8; i++) {
        int px, py;
        viewport_transform(beacons[i].x, beacons[i].y, &px, &py);
        lv_obj_t *b = lv_obj_create(bg);
        lv_obj_set_size(b, 8, 8);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x00FF00), 0);
        lv_obj_clear_flag(b, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_pos(b, px - 4, py - 4);
    }

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