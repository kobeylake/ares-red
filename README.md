# Greenhouse Environmental Monitoring System

An embedded systems project that monitors and regulates greenhouse conditions using a network of Bluetooth-enabled sensors and actuators, real-time data visualization, and responsive environmental control.

## Project Overview

This system uses a Nordic Thingy:52 sensor node and NRF52840 development boards to collect environmental data and automate vent control based on real-time conditions. It includes:

- **Environmental sensing** (temperature, humidity, light, eCO₂/TVOC)
- **Soil and distance monitoring** (moisture and ultrasonic sensors)
- **Bluetooth Low Energy (BLE)** mesh communication between nodes
- **Automated actuation** (servo-controlled vents)
- **Grafana dashboard** for historical data logging and alerts
- **Live display** on M5 Core2 for local user feedback

---

## Architecture
```
  Thingy52 --BLE--> Base Node (NRF52840DK) --BLE--> Actuator Node (NRF52840DK + CRICKIT)
                                           --USB--> Host Laptop --InfluxDB--> Grafana Dashboard
                                                                --BLE-------> M5 Core2 Viewer Display
```
---
## Project Structure
```
  /firmware/
  ├── thingy52/ # Environmental sensor logic + BLE advertising
  ├── base_node/ # BLE central node + data fusion + control logic
  ├── actuator_node/ # BLE peripheral + servo control via CRICKIT
  └── viewer_display/ # M5 Core2 LVGL display code
  
  /tools/
  ├── grafana/ # Grafana dashboard config and provisioning
  └── protocol/ # BLE message format definitions and constants
```
---

## Features

- **Bluetooth Mesh Comms**: Reliable multi-hop BLE data exchange between all nodes
-  **Sensor Integration**: Measures air quality, temperature, humidity, soil, and proximity
-  **Vent Control**: Automatically opens/closes vents using servo motors based on thresholds
-  **Grafana Dashboard**: Real-time + historical data tracking and alerting
-  **M5 Core2 UI**: Portable local display for visualising live metrics
-  **Protocol Spec**: Unified message format for cross-node communication

---

## Setup Instructions

1. **Install Zephyr SDK and dependencies**
2. **Flash firmware** to Thingy52, base node, and actuator node using `west flash`
3. **Deploy Grafana** with provided Docker compose files
4. **Power and wire hardware** as per `/docs/setup_guide.md`
5. **Run** and observe live metrics, actuator response, and dashboard logging

---

## Contributors

- James Lowe - 47442575
- Kobey Lake - 47430974
- Sam Perkins - 47429606

---
