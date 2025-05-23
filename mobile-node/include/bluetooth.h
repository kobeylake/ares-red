#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "sensor.h"

// Initialise Bluetooth stack and start advertising
void bluetooth_init(void);

// Update the advertising payload with latest sensor values
void bluetooth_update_payload(struct greenhouse_data *data);

#endif // BLUETOOTH_H
