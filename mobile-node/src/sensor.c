#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>
#include <string.h>
#include "sensor.h"

LOG_MODULE_REGISTER(sensor, LOG_LEVEL_INF);

static const struct device *temp_dev;
static const struct device *gas_dev;

static struct greenhouse_data latest_data;

void sensor_init_all(void) {
    temp_dev = DEVICE_DT_GET(DT_ALIAS(hts221));
    gas_dev  = DEVICE_DT_GET(DT_ALIAS(ccs811));

    if (!device_is_ready(temp_dev) || !device_is_ready(gas_dev)) {
        LOG_ERR("Sensors not ready");
        return;
    }

    LOG_INF("Sensors initialised");
}

void sensor_get_latest_values(struct greenhouse_data *data) {
    struct sensor_value temp, co2;

    if (sensor_sample_fetch(temp_dev) == 0 &&
        sensor_channel_get(temp_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) == 0) {
        latest_data.temperature = temp;
    }

    if (sensor_sample_fetch(gas_dev) == 0 &&
        sensor_channel_get(gas_dev, SENSOR_CHAN_CO2, &co2) == 0) {
        latest_data.co2 = co2;
    }

    *data = latest_data;
}