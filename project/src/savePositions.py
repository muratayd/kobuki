import paho.mqtt.client as mqtt
import json

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # Open the file in append mode
    with open('mqtt_data.txt', 'a') as f:
        # Write the topic and message to the file
        f.write('Topic: ' + msg.topic + '\n')
        f.write('Message: ' + str(msg.payload) + '\n')

client = mqtt.Client()
client.on_message = on_message

# Connect to the MQTT broker
client.connect("localhost", 1883, 60)

# Subscribe to the topics
client.subscribe("robot/coordinates")
client.subscribe("pozyx/position")

# Blocking call that processes network traffic, dispatches callbacks and handles reconnecting.
client.loop_forever()