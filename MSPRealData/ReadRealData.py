import serial
import re
import time
import sys
import argparse
import paho.mqtt.publish as publisher
import geocoder
import json

class DatasetReader:
    HOSTNAME = "test.mosquitto.org"

    def __init__(self):
        self.locationByIp = geocoder.ip('me')
        self.createArgumentsParser()

    def start(self):
        args = self.parser.parse_args()
        self.sendDataEveryNSeconds(args.id, args.topic, args.port)

    def createArgumentsParser(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--version", action="version", version="%(prog)s 0.0.1")
        parser.add_argument("-id", type=int, default=0)
        parser.add_argument("-t", "--topic", type=str, default="/itifermi/citta/edificio/stanza/")
        parser.add_argument("-p", "--port", type=str, default="COM4")
        self.parser = parser

    def sendDataEveryNSeconds(self, id, topic, comPort):
        self.serial = serial.Serial(port=comPort, baudrate=115200)
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
                
            #time.sleep(timeSample)

    def generateDataset(self, id):
        values = self.readData()
        data = {
            "id" : id,
            "time" : int(time.time()),
            "lat" : self.locationByIp.lat,
            "long" : self.locationByIp.lng,
            "pres" : values[2],
            "temp" : values[0],
            "hum" : values[1]
        }
        return data

    def readData(self):
        data = self.serial.read_until(terminator=b"END;", size=30).decode("utf-8")
        if re.match("^START:.*END;$", data):
            data = data.replace("START:", "")
            data = data.replace("END;", "")
            parsedData = data.split(",")
            return parsedData

    
        

if __name__ == "__main__":
    generator = DatasetReader()
    generator.start()
    
