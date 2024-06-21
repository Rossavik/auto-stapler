import tkinter as tk
from tkinter import messagebox, font as tkfont
import serial
import time
import numpy as np

def_speed = 16000.0
readline = "Serial output"
servoSpeed = 8

def calculate_nails(length):
    if length%350 != 0:
        nails = (length-length%350)/350+1
    else:
        nails = length/350
    return nails

def generate_gcode(cc, no_splines, length, speed):
    nail_length = length - 60
    nails = calculate_nails(nail_length)
    dist_nails = nail_length/nails
    y_pos=np.zeros(int(nails)+1)
    for i in range(int(nails)+1):
        y_pos[i] = i*dist_nails
    y_neg = np.flip(y_pos)
    print(y_pos)
    print(y_neg)

    feed_rate = speed*def_speed/100
    gcode = f"""G90
G21
G92 X0 Y0
"""
    gcode+=f"M220 X{speed*def_speed/100} Y{speed*def_speed/100} S{servoSpeed/speed*100}\n"
    iter=1
    for i in range(0, no_splines):
        gcode += f"G01 Y{i*cc}\n"
        gcode += f"M03 D{dwell_time}\n"
        if(iter % 2 != 0):
            gcode += f"G03 X{nail_length} Y{i*cc} S{servoSpeed}\n"
        if(iter % 2 == 0):
            gcode += f"G03 X{0} Y{i*cc} S{feed_rate}\n"
        #gcode += f"M03 D{dwell_time}\n"
        iter += 1
    gcode += "\n"
    gcode += "\n"
    gcode += "M30\n"

    return gcode

    
if __name__ == "__main__":
    cc = 66
    no_splines = 10
    length = 1500
    speed = 100
    dwell_time = 0.4

    gcode = generate_gcode(cc, no_splines, length, speed)

    #with open("nail_shooting.gcode", "w") as output_file:
    #    output_file.write(gcode)
    
    #print(f"G-code has been generated and saved as 'nail_shooting.gcode'")

def save_gcode():
    cc = int(cc_entry.get())
    no_splines = int(no_splines_entry.get())
    length = int(length_entry.get())
    speed = int(speed_entry.get())
    file_name = file_name_entry.get()

    gcode = generate_gcode(cc, no_splines, length, speed)

    with open(file_name, "w") as output_file:
        output_file.write(gcode)

    messagebox.showinfo("Success", f"G-code has been generated and saves as '{file_name}'")

def send_gcode():
    # Set the COM port and baud rate to match Arduino
    ser = serial.Serial('COM7', 9600) 
    readline = "Sending g-code"
    serial_label.config(text = readline)
    # Open the G-code file for reading
    with open('nail_shooting.gcode', 'r') as gcode_file:
    #with open('{file_name}', 'r') as gcode_file:
        for line in gcode_file:
            ser.write(line.encode())
            print(f'Sent: {line.strip()}')
            time.sleep(1)
    ser.close()


app = tk.Tk()
app.config(bg='grey')
app.title("G-kode generator stifterobot")
app.geometry("700x300") # Adjust the window size

label_font = tkfont.Font(size=12)
entry_font = tkfont.Font(size=12)
button_font = tkfont.Font(size=12)

cc_label = tk.Label(app, text="CC", font=label_font)
cc_label.grid(row=0, column=0)
cc_entry = tk.Entry(app, font=entry_font)
cc_entry.insert(0, "66")
cc_entry.grid(row=0, column=1)
cc_mm = tk.Label(app, text="mm", font=label_font)
cc_mm.grid(row=0,column=2)

no_splines_label = tk.Label(app, text="Antall spiler", font=label_font)
no_splines_label.grid(row=1, column=0)
no_splines_entry = tk.Entry(app, font=entry_font)
no_splines_entry.insert(0, "7")
no_splines_entry.grid(row=1,column=1)
no_splines_hint = tk.Label(app, text="cc * spiler kan ikke overstige 490 mm", font=label_font, anchor='w', justify='left')
no_splines_hint.grid(row=1, column=3)

length_label = tk.Label(app, text="Lengde", font=label_font)
length_label.grid(row=2, column=0)
length_entry = tk.Entry(app, font=entry_font)
length_entry.insert(0, "1500")
length_entry.grid(row=2,column=1)
length_mm = tk.Label(app, text="mm", font=label_font)
length_mm.grid(row=2,column=2)
length_hint = tk.Label(app, text="Max 2290 mm", font=label_font, anchor='w', justify='left')
length_hint.grid(row=2, column=3)

speed_label = tk.Label(app, text="Hastighet", font=label_font)
speed_label.grid(row=3,column=0)
speed_entry = tk.Entry(app, font=entry_font)
speed_entry.insert(0, "100")
speed_entry.grid(row=3,column=1)
speed_percentage = tk.Label(app, text="%", font=label_font, anchor='w', justify='left')
speed_percentage.grid(row=3, column=2)

file_name_label = tk.Label(app, text="File name:", font=label_font)
file_name_label.grid(row=4, column=0)
file_name_entry = tk.Entry(app, font=entry_font)
file_name_entry.insert(0, "nail_shooting.gcode")
file_name_entry.grid(row=4, column=1)
file_name_hint = tk.Label(app, text="husk .gcode", font=label_font, anchor='w', justify='left')
file_name_hint.grid(row=4, column=3)

space_label = tk.Label(app, text="Generate or send gcode:", font=label_font)
space_label.grid(row=5, column=0)

# Create the "Generate G-code" button
generate_button = tk.Button(app, text="Generate G-code", command=save_gcode, font=button_font)
generate_button.grid(row=6, column=1, columnspan=2)

# Create the "Send G-code" button
send_button = tk.Button(app, text="Send G-code", command=send_gcode, font=button_font)
send_button.grid(row=6, column=3, columnspan=2)

serial_label = tk.Label(app, text=readline, font=label_font)
serial_label.grid(row=11,column=0)

app.mainloop()

while True:
    ser = serial.Serial('COM7', 9600)
    readline = ser.readline()
    print(readline)
    serial_label.config(text = readline)