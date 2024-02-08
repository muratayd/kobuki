import RPi.GPIO as GPIO
import time
import paho.mqtt.client as mqtt

# Set GPIO Pins
GPIO_TRIGGER = 18
GPIO_ECHO = 24

# Set the GPIO modes
GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)

def distance():

    # Send a pulse
    GPIO.output(GPIO_TRIGGER, True)
    time.sleep(0.00001)
    GPIO.output(GPIO_TRIGGER, False)

    start_time = time.time()
    stop_time = time.time()

    # Save start time
    while GPIO.input(GPIO_ECHO) == 0:
        start_time = time.time()

    # Save time of arrival
    while GPIO.input(GPIO_ECHO) == 1:
        stop_time = time.time()

    # Time difference between start and arrival
    time_elapsed = stop_time - start_time
    # Multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (time_elapsed * 34300) / 2

    return distance

def on_publish(client, userdata, result):
    print("Data published \n")
    pass

# MQTT Settings
broker_address = "localhost"  # Use your MQTT broker address
port = 1883
topic = "sensor/distance"

client = mqtt.Client("P1")  # create new instance
client.on_publish = on_publish  # assign function to callback
client.connect(broker_address, port)  # connect to broker

try:
    while True:
        dist = distance()
        print("Measured Distance = %.1f cm" % dist)
        client.publish(topic, str(dist))
        time.sleep(0.2)  # Publish at 5Hz

except KeyboardInterrupt:
    print("Measurement stopped by User")
    GPIO.cleanup()