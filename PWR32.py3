import serial
from serial import SerialException
import time
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, FigureCanvasAgg
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.figure import Figure
import numpy as np
from datetime import date

import serial.tools.list_ports

import PySimpleGUI as sg
import os.path



# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# declaring some important variables


data    = []
datat   = []

Acquisition = False
Connected   = False
Rolling     = False

initial_t   = time.time()
today       = date.today()

readFreq    = 2
rollSize    = 200

filename        = ""
tooltip_logo    = 'Developed by Bruno P. Gelli, Aug 2022.\nGit: url'
tooltip_start   = 'Only active once device is connected'
tooltip_stop    = 'Only active once device is connected'
tooltip_sdata   = 'Only active when acquisition is stopped'
tooltip_sgraph  = 'Only active when acquisition is stopped'


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
#functions

#define a important funcion for drawing
def draw_figure(canvas, figure, loc=(0, 0)):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

#updates de graph
def graph_update(axis, time, measurement, figure_handler, size):
    axis.cla()
    axis.grid()
    axis.set_xlabel("Time (s)")
    axis.set_ylabel("Pressure (mBar)")
    if Rolling:
        time = time[-size:]
        measurement = measurement[-size:]
    axis.plot(time, measurement,  color='b')
    figure_handler.draw()

#writes to the serial buss
def write_data(x):
    arduino.write(bytes(x, 'utf-8'))
    return x

#reads the serial buss
def read_data():
    measured = arduino.readline().decode('utf-8').rstrip()
    return measured

#calculates the enlapsed time since X and return a string
def enlaps(X):
    enlapsed = time.time() - X
    hours = round(enlapsed/3600,0)
    minut = round((enlapsed%3600)/60,0) 
    secon = round((enlapsed%3600)%60,2)
    return str("["+str('{:02.0f}'.format(hours))+":"+str('{:02.0f}'.format(minut))+":"+str('{:04.1f}'.format(secon))+"]")

#check the serial ports
def serial_ports(lista):
    lista.clear()
    ports = serial.tools.list_ports.comports()

    for port, desc, hwid in sorted(ports):
            lista.append(port +": "+ desc)

    return lista

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# defining the list of serial ports (Perhaps could be done in an event)

COM_list = []
COM_list = serial_ports(COM_list)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# layout

sg.theme('DarkGrey1')    # Keep things interesting for your users

# first column
file_list_column = [
    [
        sg.Image('logo.png', subsample=3, tooltip=tooltip_logo),
    ],
    [
        sg.Text("Choose the COM port where the device is:"),
    ],
    [
        sg.Combo(COM_list,size=(49, 1), default_value="Select the port", enable_events=True, key="-COM-"),
        sg.Button('Refresh'),
    ],
    [
        sg.Text("Once sure of the selection, connect the device:"),
    ],
    [
        sg.Text("",size=(44, 1), background_color='white', text_color='black', enable_events=True, key="-CON FEEDBACK-"),
        sg.Button('Connect'),
    ],
    [
        sg.Text("Device's log output:"),
    ],
    [
        sg.Multiline("Log output \n",size=(58, 10), autoscroll = True, background_color='white', text_color='black', enable_events=True, key="-LOG-"),
    ], 
    [
        sg.Text("Sampling frequency ("+str(readFreq)+ " Hz)", key="-textfreq-", enable_events=True),
    ],
    [
        sg.Slider(range=(0,20), default_value=readFreq, disable_number_display = True, resolution=.1,  orientation='horizontal', expand_x=True, enable_events=True, key="-slider-"),
    ], 
    [
        sg.Text("Rolling size ("+str(rollSize)+ " points)", key="-textroll-", enable_events=True),
    ],
    [
        sg.Slider(range=(10,2000), default_value=rollSize, disable_number_display = True,  resolution=10,  orientation='horizontal', expand_x=True, enable_events=True, key="-rolling-"),
        sg.Checkbox("Rolling", default = False, enable_events=True, expand_y=True,key="-checkbox-"),
    ], 
    [
        sg.Button('Start Acquisition',pad=(30,10), expand_x=True, tooltip=tooltip_start),
        sg.Button('Stop Acquisition',pad=(30,10), expand_x=True, tooltip=tooltip_stop),
    ],     
    [
        sg.HSeparator(pad=(3,17))
    ],
    [
        sg.Text("Save acquired data:"),
    ],
    [     
        sg.FolderBrowse(),
        sg.In(default_text = str(today.strftime("%b-%d-%Y")),size=(25, 1), enable_events=True, key="-Save data FOLDER-"),
        sg.Button('Save data', tooltip=tooltip_sdata),
        sg.Button('Save graph', tooltip=tooltip_sgraph),
    ],
]

# second column
image_viewer_column = [
    [
        sg.Canvas(size=(1280, 720), key="-CANVAS-"),
    ],
]

# combining the layouts
layout = [
    [
        sg.Column(file_list_column),
        sg.VSeperator(),
        sg.Column(image_viewer_column),
    ]
]

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# defining the plot and starting the timmer

window = sg.Window("PWR32 host software", layout, size=(1780,750), resizable=True, finalize=True)

canvas_elem = window['-CANVAS-']
canvas = canvas_elem.TKCanvas

fig = Figure(figsize=(16, 9), dpi=80)
ax = fig.add_subplot(111)
ax.set_xlabel("Time (s)")
ax.set_ylabel("Pressure (mBar)")
ax.grid()
fig_agg = draw_figure(canvas, fig)

old_t = time.time()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# Run the Event Loop
while True:

    event, values = window.read(timeout=10)

    delta = time.time() - old_t

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Arduino talk
    if Acquisition and delta > (1/readFreq):
        if arduino.in_waiting == 0:
            write_data('1')

        if arduino.in_waiting > 0:
            measurement = read_data()
            data.append(float(measurement))
            datat.append(time.time() - initial_t)
            window["-LOG-"].update(enlaps(initial_t)+" - measured "+ str(measurement)+".\n" , append=True)
            graph_update(ax, datat, data, fig_agg, rollSize)
            old_t = time.time()
       
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# closing the window
    if event == "Exit" or event == sg.WIN_CLOSED:
        arduino.close()
        break

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the refresh button
    elif event == "Refresh": 
        COM_list = serial_ports(COM_list)
        window["-COM-"].update(values = COM_list, value="Select the port")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the connect button
    elif event == "Connect": 
        try:
            arduino = serial.Serial(port=values["-COM-"].split(":")[0], baudrate=115200, timeout=.05)

        except SerialException:
            window["-CON FEEDBACK-"].update("Something went wrong when oppening '"+ values["-COM-"].split(":")[0] +"'\n")

        else:
            window["-CON FEEDBACK-"].update("Successful connection at: "+ values["-COM-"].split(":")[0])
            window["-LOG-"].update(enlaps(initial_t)+" - Successful connection at: "+ values["-COM-"].split(":")[0] +"\n" , append=True)
            Connected = True

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the Start Acquisition button
    elif event == "Start Acquisition" and Connected and not Acquisition: 
        Acquisition = True
        readFreq = values["-slider-"]
        window["-LOG-"].update(enlaps(initial_t)+" - Acquisition started ("+str(readFreq)+" Hz, "+str('{:0.2f}'.format(1/readFreq))+" s) \n" , append=True)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the Stop Acquisition button
    elif event == "Stop Acquisition" and Connected and Acquisition: 
        Acquisition = False
        window["-LOG-"].update(enlaps(initial_t)+" - Acquisition stopped. ("+str(len(data))+" points) \n" , append=True)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the Rolling checkbox
    elif event == "-checkbox-": 
        Rolling  = values["-checkbox-"]
        rollSize = int(values["-rolling-"])
        if not Acquisition:
            graph_update(ax, datat, data, fig_agg, rollSize)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
# Behaviour of the frequency slider
    elif event == "-slider-": 
        window["-textfreq-"].update("Sampling frequency ("+str(values["-slider-"])+ " Hz)")
  
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
# Behaviour of the rolling slider
    elif event == "-rolling-": 
        window["-textroll-"].update("Rolling size ("+str('{:.0f}'.format(values["-rolling-"]))+ " points)")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the Save data button
    elif event == "Save data" and not Acquisition: 
        if data:
            filename = str(values["-Save data FOLDER-"]+".txt")
            f = open(filename, "w")
            f.write("measurements:\n")
            for item in data:
                f.write(str(item)+"\n")
            f.close()
            window["-LOG-"].update(enlaps(initial_t)+" - Data saved to "+ filename +" \n" , append=True)

        else:
            window["-LOG-"].update(enlaps(initial_t)+" - Data is empty. Try after acquiring data!\n" , append=True)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
# Behaviour of the Save graph button
    elif event == "Save graph" and not Acquisition: 
        if data:
            filename = str(values["-Save data FOLDER-"]+".png")
            fig.savefig(filename)
            window["-LOG-"].update(enlaps(initial_t)+" - Figure saved to "+ filename +" \n" , append=True)

        else:
            window["-LOG-"].update(enlaps(initial_t)+" - Data is empty. Try after acquiring data!\n" , append=True)



window.close()