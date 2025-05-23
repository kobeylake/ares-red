#include "sensor.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(sensor_module, LOG_LEVEL_INF);

static const struct device *temp_dev;
static const struct device *gas_dev;

struct greenhouse_data ghs_data;
struct k_mutex ghs_mutex;

#define SENSOR_STACK_SIZE 1024
#define SENSOR_PRIORITY 5
K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_STACK_SIZE);
static struct k_thread sensor_thread_data;

static void sensor_thread_fn(void *a, void *b, void *c) {
    struct sensor_value temp, co2, humid;

    while (1) {
        if (sensor_sample_fetch(temp_dev) == 0 &&
            sensor_channel_get(temp_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) == 0 &&
            sensor_channel_get(temp_dev, SENSOR_CHAN_HUMIDITY, &humid) == 0 &&
            sensor_sample_fetch(gas_dev) == 0 &&
            sensor_channel_get(gas_dev, SENSOR_CHAN_CO2, &co2) == 0) {

            k_mutex_lock(&ghs_mutex, K_FOREVER);
            ghs_data.temperature = temp;
            ghs_data.humidity = humid;
            ghs_data.co2 = co2;
            k_mutex_unlock(&ghs_mutex);

            LOG_INF("Temp: %d.%06d°C | Humidity: %d.%06d%% | CO₂: %dppm",
                    temp.val1, temp.val2,
                    humid.val1, humid.val2,
                    co2.val1);
        } else {
            LOG_ERR("Sensor fetch failed");
        }

        k_sleep(K_SECONDS(5));
    }
}

void sensor_task_start(void) {
    temp_dev = DEVICE_DT_GET(DT_ALIAS(hts221));
    gas_dev  = DEVICE_DT_GET(DT_ALIAS(ccs811));

    if (!device_is_ready(temp_dev) || !device_is_ready(gas_dev)) {
        LOG_ERR("Sensors not ready");
        return;
    }

    k_mutex_init(&ghs_mutex);

    k_thread_create(&sensor_thread_data, sensor_stack, SENSOR_STACK_SIZE,
                    sensor_thread_fn, NULL, NULL, NULL,
                    SENSOR_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&sensor_thread_data, "sensor_thread");

    LOG_INF("Sensor thread started");
}
