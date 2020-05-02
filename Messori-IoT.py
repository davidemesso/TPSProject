import random
import time
import sys
import argparse
import paho.mqtt.publish as publisher
import geocoder
import json

class DatasetGenerator:
    HOSTNAME = "test.mosquitto.org"

    def __init__(self):
        self.locationByIp = geocoder.ip('me')
        self.createArgumentsParser()

    def start(self):
        args = self.parser.parse_args()
        self.sendDataEveryNSeconds(args.id, args.topic, args.tsamp)

    def createArgumentsParser(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--version", action="version", version="%(prog)s 0.0.1")
        parser.add_argument("-id", type=int, default=0)
        parser.add_argument("-t", "--topic", type=str, default="/itifermi/citta/edificio/stanza/")
        parser.add_argument("-ts", "--tsamp", type=int, default=20)
        self.parser = parser

    def sendDataEveryNSeconds(self, id, topic, timeSample):
        while True:
            data = json.dumps(self.generateDataset(id)).encode("utf-8")

            print(data)
            try:
                publisher.single(
                    topic=topic,
                    payload=data,
                    qos=0,
                    retain=True,
                    hostname=self.HOSTNAME,
                    port=1883)
            except:
                print("Unable to publish data")
                
            time.sleep(timeSample)

    def generateDataset(self, id):
        data = {
            "id" : id,
            "time" : int(time.time()),
            "lat" : self.locationByIp.lat,
            "long" : self.locationByIp.lng,
            "pres" : self.generatePressure(),
            "temp" : self.generateTemperature(),
            "hum" : self.generateHumidity()
        }
        return data

    def generatePressure(self):
        intPart = random.randint(990, 1020)
        decPart = random.randint(0, 99)
        return f'{intPart}.{decPart}'

    def generateTemperature(self):
        intPart = random.randint(18, 22)
        decPart = random.randint(0, 99)
        return f'{intPart}.{decPart}'

    def generateHumidity(self):
        return random.randint(60, 90)
        

if __name__ == "__main__":
    generator = DatasetGenerator()
    generator.start()
    
