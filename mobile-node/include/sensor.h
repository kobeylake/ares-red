#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

struct greenhouse_data {
    struct sensor_value temperature;
    struct sensor_value co2;
    struct sensor_value humidity;
};

extern struct greenhouse_data ghs_data;
extern struct k_mutex ghs_mutex;

void sensor_task_start(void);

#endif // SENSOR_H
