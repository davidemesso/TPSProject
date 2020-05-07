# MSP real data

#### This part handle both the acquisition of real time data from BME sensor using MSP440F5529, and the reading of this data sent from the microcontroller on serial using a python script, sending them to MQTT broker the same way as in the simulated version

The C program running on the MSP uses FreeRTOS real time operating system to handle multitasking, it reads temperature, humidity and pressure data from a BME280 sensor.
The data are sent over I2C channel from BME to MSP, then the MSP create a packet and send them over serial, then the python script can manipulate them

Note: in this version cant be changed from python script but it's managed by the program on the MSP

Consult [Serial data format](#Serial-data-format)

### Serial data format
This is the format of data sent over serial
```
packet:
START:temp,hum,presEND;

single data format:
temp: dd.ddd (°C)
hum:  ddd    (%)
pres: dddd   (hPa)
```
The keywords START and END; are used to easly identify a single packet in a long serial stream, and also to handle interrupted or late started connections
The real payload is what's between the keywords, which are the values that we want comma separetad

Note: not significant figures may not be present, so the total payload lenght may vary after them