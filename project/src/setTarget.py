import paho.mqtt.client as mqtt
import json
import time

# MQTT Broker settings
broker_address = "localhost"  # or "broker.hivemq.com" for a public broker
broker_port = 1883
topic = "robot/target"

# Target coordinates
target_x = 1.0
target_y = 0.5

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Publishing a message to a topic
    client.publish(topic, json.dumps({"x": target_x, "y": target_y}), qos=1)
    print(f"Message published to topic {topic}")
    # Disconnect after publishing
    client.disconnect()

# The callback for when the client disconnects from the server.
def on_disconnect(client, userdata, rc):
    print("Disconnected from the broker")
    # End the script by stopping the loop
    client.loop_stop()

# Create a MQTT Client instance
client = mqtt.Client()

# Assign callback functions
client.on_connect = on_connect
client.on_disconnect = on_disconnect

# Connect to the MQTT Broker
client.connect(broker_address, broker_port, 60)

# Start the loop
client.loop_start()

# Wait a bit for publishing and disconnecting
time.sleep(2)
