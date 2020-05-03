# IoT Project

In this project:

- [IoT Project](#iot-project)
  - [Sensor part](#sensor-part)
    - Simulation
    - MSP real data
  - [Backend script part](#backend-script-part)
    - MQTT subscriber
    - MySQL handler
    - WS communication
  - [Tornado server part](#tornado-server-part)
    - WEB visualization
    - Storic data request
    - Real time data communication



## Sensor part

Where the data are acquired, it could be done by real sensors and sent on serial port by [MSP430 program](), or it could be simulated by a [python script](Messori-IoT.py).
Data must be sent in MQTT to a broker, which temporally handle the communication between sensors and backend script.

Consult [Data Format](#Data-format)

## Backend script part

[This script](Messori-Backend-Data-Handler.py) must subscribe to MQTT topics where the data were sent, then it stores them into MySQL server and send in real time via WebSocket to Tornado server.
This script also handle on the same WS channel incoming request of storic data, sending the SELECT query result back to the client

More information about SQL server [here](/DatabaseScheme)

## Tornado server part

The server provides a web page usefull to visualize and plot the real time data coming from sensors, it also provide a GUI to select starting date and ending date for a storic data request

Consult [WS packets format](#WS-packets-format)

More information about Tornado server [here](/TornadoServer)

### Data Format
This is the format of data sent to MQTT broker
```
{
  "id": dddd, 
  "time": epochFormat, 
  "lat": dd.dddd, 
  "long": dd.dddd, 
  "pres": dddd.dd (hPa), 
  "temp": dd.dd (°C), 
  "hum": dd (%)
}
```
Note: this can be slightly modified for different packets payload in ws communication, this standard is related only to MQTT communication

### WS packets format
TO BE IMPLEMENTED 
