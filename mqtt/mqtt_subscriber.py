# mqtt_subscriber.py
import paho.mqtt.client as mqtt

# MQTT settings
BROKER_IP = "10.89.186.37"
TOPIC = "greenhouse/data"

def on_connect(client, userdata, flags, rc):
    print(f"[MQTT] Connected with result code {rc}")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    print(f"[MQTT] Received: {msg.payload.decode()}")

def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_IP, 1883, 60)
    client.loop_forever()

if __name__ == "__main__":
    main()
