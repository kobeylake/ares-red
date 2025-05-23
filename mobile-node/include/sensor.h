#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/drivers/sensor.h>

struct greenhouse_data {
    struct sensor_value temperature;
    struct sensor_value co2;
};

void sensor_init_all(void);
void sensor_get_latest_values(struct greenhouse_data *data);

#endif // SENSOR_H
