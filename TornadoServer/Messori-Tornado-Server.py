import tornado.ioloop
import tornado.web
import tornado.websocket
import time
import json
from tornado import gen

class MainHandler(tornado.web.RequestHandler):
	def get(self):
		self.render("index.html")

class DataHandler(tornado.web.RequestHandler):
	def get(self):
		self.render("data.html")

class SensorsHandler(tornado.web.RequestHandler):
	def get(self):
		self.render("sensors.html")

class DataWsHandler(tornado.websocket.WebSocketHandler):
	connections = []
	server = None
	lastRequest = 0
	requestDict = {}
	
	def open(self):
		print("ws connected")
		self.connections.append(self)

	def on_message(self, message):
		packet = json.loads(message)
		if packet['type'] == "RealTimeData":
			self.realTimeData(packet['payload'])
		elif packet['type'] == "ServerHandshake":
			DataWsHandler.server = self
		elif packet['type'] == "StoricDataRequest":
			self.storicDataRequest(packet)
		elif packet['type'] == "StoricDataServe":
			self.storicDataServe(packet['payload'])
		elif packet['type'] == "SensorsDataRequest":
			self.sensorsDataRequest(packet)
		elif packet['type'] == "SensorsDataServe":
			self.storicDataServe(packet['payload'])

	def on_close(self):
		print("ws disconnected")
		self.connections.remove(self)

	def realTimeData(self, message):
		for connection in self.connections:
			if connection is not self:
				connection.write_message(message)

	def storicDataRequest(self, message):
		self.lastRequest+=1
		packet = message 
		packet['payload']['id'] = self.lastRequest
		packet = json.dumps(packet)
		DataWsHandler.server.write_message(packet)
		self.requestDict[self.lastRequest] = self

	def storicDataServe(self, message):
		packet = json.dumps(message['data'])
		self.requestDict[message['id']].write_message(packet)
		self.requestDict[message['id']] = None

	def sensorsDataRequest(self, message):
		self.lastRequest+=1
		packet = message 
		packet['payload']['id'] = self.lastRequest
		packet = json.dumps(packet)
		DataWsHandler.server.write_message(packet)
		self.requestDict[self.lastRequest] = self

	def sensorsDataServe(self, message):
		packet = json.dumps(message['data'])
		self.requestDict[message['id']].write_message(packet)
		self.requestDict[message['id']] = None

class TornadoServer:
	def __init__(self, port):
		self.port = port
		
	def make_app(self):
		handlers = [
			(r"/", MainHandler),
			(r"/data", DataHandler),
			(r"/sensors", SensorsHandler),
			(r"/data/ws", DataWsHandler)
		]

		settings = {
			"static_path": "./"
		}


		return tornado.web.Application(handlers, **settings)

			
	def startServer(self):
		app = self.make_app()
		app.listen(self.port)
		try:
			tornado.ioloop.IOLoop.current().start()
		except KeyboardInterrupt:
			print("\nserver stopped... bye")

			
if __name__ == '__main__':
	server = TornadoServer(8888)
	server.startServer()