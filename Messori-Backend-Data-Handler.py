import paho.mqtt.client as client
import json
import mysql.connector as connector
import time
import websocket
import threading
import time

class MySQLHandler():
    DEFAULT_CONNECTOR_CONFIG = {
        'user': 'root',
        'password': 'rootroot',
        'host': '127.0.0.1',
        'database': 'iotdatabase'
    }

    def connect(self, config = DEFAULT_CONNECTOR_CONFIG):
        try:
            self.connector = connector.connect(**config)
            self.cursor = self.connector.cursor()
        except:
            print("database connection error")

    def executeSingleQuery(self, query):
        try:
            self.cursor.execute(query)
            self.connector.commit()
            print("updated database")
        except:
            print("Error updating database")

    def executeSingleSelectQuery(self, query):
        try:
            self.cursor.execute(query)
            return self.cursor.fetchall()
        except:
            print("Error getting data")

    def closeConnection(self):
        self.cursor.close()
        self.connector.close()        


class wsMessageReceiverThread(threading.Thread):
    def __init__(self, mySQLHandler, wsHandler):
        threading.Thread.__init__(self)
        self.mySQLHandler = mySQLHandler
        self.wsHandler = wsHandler

        self.sendServerHandshake()

    def sendServerHandshake(self):
        message = { "type": "ServerHandshake" }
        try:
            self.wsHandler.send(json.dumps(message))
        except:
            print("Tornado is not connected")

    def run(self):
        self.receiveDataWS()

    def receiveDataWS(self):
        while True:
            try:
                data = json.loads(self.wsHandler.recv())

                query = f"""SELECT * from iotData 
                            WHERE date(time) >= "{data['startTime']}" and
                                  date(time) <= "{data['endTime']}" """
                storicData = self.mySQLHandler.executeSingleSelectQuery(query)
                formattedData = []
                for el in storicData:
                    formattedData.append({
                        "id"   : str(el[0]),
                        "time" : str(el[1]),
                        "pres" : str(el[2]),
                        "temp" : str(el[3]),
                        "hum"  : str(el[4])
                    })
                packet = {
                    "type" : "StoricDataServe",
                    "payload" : {
                        "data" : formattedData,
                        "id" : data['id']
                    }
                }
                self.wsHandler.send(json.dumps(packet))
            except:
                print('Connection closed')
                break;

    

class MQTTSubscriber:
    HOSTNAME = "test.mosquitto.org"
    TOPIC = "/itifermi/#"

    def __init__(self, name):
        self.client = client.Client(name)    
        
        #MQTT related init
        self.client.on_message = self.onMessage

        #Database connection related inits
        self.mySQLHandler = MySQLHandler()
        self.mySQLHandler.connect()
        self.sensorIDList = self.getExistingSensorIDs();

        #WS sender related init
        self.wsHandler = self.openWsConnectionToTornado()

        #WS receiver related inits
        self.wsReceiver = wsMessageReceiverThread(self.mySQLHandler, self.wsHandler)
        self.wsReceiver.start()
        
    def start(self):
        self.client.connect(self.HOSTNAME)
        self.client.subscribe(self.TOPIC)
        self.client.loop_forever()

    def onMessage(self, client, userdata, message):
        data = json.loads(message.payload.decode("utf-8"))
            
        datetime = self.getDatetimeFromEpoch(data['time'])

        sensorExists = data['id'] in self.sensorIDList
        if not sensorExists:
            query = f"""INSERT INTO sensorData (id, lat, lng, topic)
                        VALUES ({data['id']}, 
                                {data['lat']}, {data['long']}, 
                                "{message.topic}")"""
            
            self.mySQLHandler.executeSingleQuery(query)
            self.sensorIDList.append(data['id'])
            print("New sensor inserted")
        
        query = f"""INSERT INTO iotData (id, time, press, temp, hum) 
                    VALUES ({data['id']}, "{datetime}", 
                            {data['pres']}, {data['temp']}, 
                            {data['hum']})"""

        self.mySQLHandler.executeSingleQuery(query)
        self.sendDataToTornado(json.dumps(data))

    def getDatetimeFromEpoch(self, epoch):
        localtime = time.localtime(epoch)
        return time.strftime('%Y-%m-%d %H:%M:%S', localtime)

    def getExistingSensorIDs(self):
        IDlist = []
        
        query = f"""SELECT id from sensorData"""
        for id in self.mySQLHandler.executeSingleSelectQuery(query):
            IDlist.append(id[0])

        return IDlist

    def openWsConnectionToTornado(self):
        ws = None
        try:
            ws = websocket.create_connection("ws://127.0.0.1:8888/data/ws")
        except:
            print("ws connection error")
        return ws


    def sendDataToTornado(self, data):
        try:
            packet = {
                "type" : "RealTimeData",
                "payload" : str(data)
            }
            self.wsHandler.send(json.dumps(packet))
        except:
            self.wsHandler.close()
            print("error sending data")


if __name__ == '__main__':
    subscriber = MQTTSubscriber("")
    subscriber.start()
    