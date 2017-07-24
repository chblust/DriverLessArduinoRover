#  Copyright © 2017 Chris Blust. All rights reserved.
#  IntelHacks 2017 Judges granted any rights desired for judging


from gps3 import gps3
from bluepy import btle
import time

#function that checks for nan value returned from gps
def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False
try:
    #setup gps
    gps_socket = gps3.GPSDSocket()
    data_stream = gps3.DataStream()
    gps_socket.connect()
    gps_socket.watch()
    
    #connect to the arduino 101
    arduino = btle.Peripheral("84:68:3E:00:07:AD");
    
    #get the Bluetooth Low Energy service and characteristic
    serviceUUID = btle.UUID("7890")
    trackUUID = btle.UUID("7890")
    service = arduino.getServiceByUUID(serviceUUID)
    oriChar = service.getCharacteristics(trackUUID)[0]
    
    print("Connected")
    
    #variable that ensures the track angle is only written to the characteristic once
    written = False
    
    #sometimes the gps will return non, so this loop is here so it keeps trying until it isn't nan
    for new_data in gps_socket:
        if new_data:
            data_stream.unpack(new_data)
            
            track = data_stream.TPV['track']
            
            if is_number(track) and not written:
                oriChar.write(bytes(" ", 'UTF-8'))
                oriChar.write(bytes(str(track), 'UTF-8'))
                print(track)
                written = True





except KeyboardInterrupt:
    print("Done!")
