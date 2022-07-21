import paho.mqtt.client as mqtt
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import json
import paho.mqtt.publish as publish

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("topic_sub")

def on_message(client, userdata, msg):
    message = {}
    message['message'] = str(msg.payload)[2:-1]
    messageJson = json.dumps(message)
    myAWSIoTMQTTClient.publish("topic_sub", messageJson, 1)

def customCallback(client, userdata, message):
    jsonmessage = json.loads(message.payload)
    publish.single("topic_pub", jsonmessage["message"], hostname="localhost", port=1886, client_id="2")

host ="a3oupf4v1n71r-ats.iot.eu-west-1.amazonaws.com"
rootCAPath = "root-CA.crt"
certificatePath = "super_bin.cert.pem"
privateKeyPath = "super_bin.private.key"
clientId = "basicPubSub"
port=8883

myAWSIoTMQTTClient = None
myAWSIoTMQTTClient = AWSIoTMQTTClient("TransparentBridge")
myAWSIoTMQTTClient.configureEndpoint(host, 8883)
myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath) 
myAWSIoTMQTTClient.configureConnectDisconnectTimeout(10) 
myAWSIoTMQTTClient.configureMQTTOperationTimeout(5)
myAWSIoTMQTTClient.connect()


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

myAWSIoTMQTTClient.subscribe("topic_pub", 1, customCallback)

client.connect("localhost", 1886, 60)
client.loop_forever()