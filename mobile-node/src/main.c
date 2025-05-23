#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor.h"
#include "bluetooth.h"

LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

int main(void) {
    LOG_INF("Greenhouse Thingy52 starting...");

    sensor_task_start();       // Start threaded sensor sampling
    bluetooth_init();          // Initialise BLE
    bluetooth_task_start();    // Start threaded BLE advertiser

    // Main thread sleeps forever — all work is done in threads
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
