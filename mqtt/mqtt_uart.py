import serial
import paho.mqtt.client as mqtt

BROKER_IP = "10.89.186.37"
PORT = 1883
TOPIC = "greenhouse/data"
UART_PORT = "COM7"
BAUD_RATE = 115200

def main():
    ser = serial.Serial(UART_PORT, BAUD_RATE, timeout=1)
    client = mqtt.Client()
    client.connect(BROKER_IP, PORT, 60)

    while True:
        line = ser.readline().decode('utf-8').strip()
        if line.startswith("{") and line.endswith("}"):
            print("Publishing:", line)
            client.publish(TOPIC, line)

if __name__ == "__main__":
    main()
