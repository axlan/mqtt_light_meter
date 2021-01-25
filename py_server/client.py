import paho.mqtt.client as mqtt
from playsound import playsound
import struct

fd = open('data.bin' ,'wb')


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("outTopic")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    size = int(len(msg.payload) / 2)
    data = struct.unpack(f'{size}h', msg.payload)
    # print(msg.topic+" "+str(msg.payload))
    low = min(data)
    print(low, max(data))
    if (low < 83):
        playsound('beep-09.mp3')
    fd.write(msg.payload)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.1.110", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
