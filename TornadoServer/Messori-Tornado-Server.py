import tornado.ioloop
import tornado.web
import tornado.websocket
import time
from tornado import gen

class MainHandler(tornado.web.RequestHandler):
	def get(self):
		self.render("index.html")

class DataHandler(tornado.web.RequestHandler):
	def get(self):
		self.render("data.html")

class DataWsHandler(tornado.websocket.WebSocketHandler):
	connections = []
	
	def open(self):
		print("ws connected")
		self.connections.append(self)

	def on_message(self, message):
		print("msg received")
		for connection in self.connections:
			if connection is not self:
				connection.write_message(message)

	def on_close(self):
		print("ws disconnected")
		self.connections.remove(self)

class TornadoServer:
	def __init__(self, port):
		self.port = port
		
	def make_app(self):
		handlers = [
			(r"/", MainHandler),
			(r"/data", DataHandler),
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