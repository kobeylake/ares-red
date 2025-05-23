#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor.h"

void bluetooth_init(void);
void bluetooth_update_payload(struct greenhouse_data *data);

LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

void main(void) {
    LOG_INF("Greenhouse Thingy52 starting...");

    sensor_init_all();
    bluetooth_init();

    while (1) {
        struct greenhouse_data data;
        sensor_get_latest_values(&data);
        bluetooth_update_payload(&data);
        k_sleep(K_SECONDS(5));
    }
}
