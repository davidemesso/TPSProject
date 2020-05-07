import serial

ser = serial.Serial(
    port='COM4',\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
        timeout=0)

while True:
    data = ser.read_until(expected= "END;", size = 30).decode("utf-8")
    if data:
        print(data)