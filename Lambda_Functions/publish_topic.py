import json
import boto3

client = boto3.client('iot-data', region_name='eu-west-1')

def lambda_handler(event, context):
    value = event['message']
    response = client.publish(
        topic='topic_pub',
        qos=1,
        payload=json.dumps({"message":value})
    )
    
    return {
        'statusCode': 200,
        'body': json.dumps('Hello from Lambda!')
    }