#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "sensor.h"

// Initialise Bluetooth stack and start advertising
void bluetooth_init(void);

// Start BLE advertiser thread (added for background BLE updates)
void bluetooth_task_start(void);

// Update the advertising payload with latest sensor values (optional if used elsewhere)
void bluetooth_update_payload(struct greenhouse_data *data);

#endif // BLUETOOTH_H
