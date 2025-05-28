# mqtt_uart.py
import time
import json
import paho.mqtt.client as mqtt

# MQTT settings
BROKER_IP = "172.19.41.241"
TOPIC = "greenhouse/data"

# Simulated data
test_data = {
    "temp": 25.7,
    "humidity": 45.2,
    "co2": 850,
    "yaw": 12.5,
    "angle": 33.0
}

def main():
    client = mqtt.Client()
    client.connect(BROKER_IP, 1883, 60)

    # Convert to JSON and publish
    json_data = json.dumps(test_data)
    client.publish(TOPIC, json_data)
    print(f"[MQTT] Published: {json_data}")

    client.disconnect()

if __name__ == "__main__":
    while True:
        main()
        time.sleep(5)  # Send every 5 seconds
