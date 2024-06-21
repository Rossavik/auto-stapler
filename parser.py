import serial
import time

# Set the COM port and baud rate to match Arduino
ser = serial.Serial('COM7', 9600) 

# Open the G-code file for reading
with open('nail_shooting.gcode', 'r') as gcode_file:
    for line in gcode_file:
        ser.write(line.encode())
        print(f'Sent: {line.strip()}')
        time.sleep(0.1)

ser.close()