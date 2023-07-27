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

dataCh0 = []
dataCh1 = []
dataCh2 = []
dataCh3 = []
timeCh0 = []
timeCh1 = []
timeCh2 = []
timeCh3 = []

Acquisition = False
Connected   = False
Rolling     = False
LogScale    = False
Written     = False

initial_t   = time.time()
today       = date.today()

readFreq    = 2
rollSize    = 200

Channel     = 0;

filename        = ""
tooltip_logo    = 'Developed by Bruno Gelli, Aug 2022.\nMore information at github.com/BrunoGelli/PWR32'
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
def graph_update(axis, time, measurement, figure_handler, size, xlabel, ylabel, title, C):
    axis.cla()
    axis.grid()
    axis.set_title(title)
    axis.set_xlabel(xlabel)
    axis.set_ylabel(ylabel)
    if Rolling:
        time = time[-size:]
        measurement = measurement[-size:]

    if LogScale:
        axis.set_yscale("log")
    else:
        axis.set_yscale("linear")

    axis.plot(time, measurement,  color=C)
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
    return str("["+str('{:02.0f}'.format(hours))+":"+str('{:02.0f}'.format(minut))+":"+str('{:03.1f}'.format(secon))+"]")

#check the serial ports
def serial_ports(lista):
    lista.clear()
    ports = serial.tools.list_ports.comports()

    for port, desc, hwid in sorted(ports):
            lista.append(port +": "+ desc)

    return lista

#calculates the pressure
def volts_to_pressure(volts):
    volts = float(volts)
    return pow(10,(volts/0.6)-12)    

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
        sg.Combo(COM_list,size=(39, 1), default_value="Select the port", enable_events=True, key="-COM-"),
        sg.Button('Refresh'),
    ],
    [
        sg.Text("Once sure of the selection, connect the device:"),
    ],
    [
        sg.Text("",size=(33, 1), background_color='white', text_color='black', enable_events=True, key="-CON FEEDBACK-"),
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
        sg.Checkbox("LogScale", default = False, enable_events=True, key="-checklog-"),
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
        sg.FolderBrowse(target="-Save data FOLDER-"),
        sg.In(default_text = "\\"+str(today.strftime("%b-%d-%Y")),size=(25, 1), enable_events=True, key="-Save data FOLDER-"),
        sg.Button('Save data', tooltip=tooltip_sdata),
        sg.Button('Save graph', tooltip=tooltip_sgraph),
    ],
]

# second column
image_viewer_column = [
    [
        sg.Canvas(size=(1230, 900), key="-CANVAS0-"),
    ],    
    [
        sg.Canvas(size=(1230, 900), key="-CANVAS1-"),
    ],
]

# third column
image_viewer_column2 = [
    [
        sg.Canvas(size=(300, 500), key="-CANVAS2-"),
    ],    
    [
        sg.Canvas(size=(300, 500), key="-CANVAS3-"),
    ],
]

# combining the layouts
layout = [
    [
        sg.Column(file_list_column),
        sg.VSeperator(),
        sg.Column(image_viewer_column),
        sg.VSeperator(),
        sg.Column(image_viewer_column2),
    ]
]

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# defining the plot and starting the timmer

window = sg.Window("PWR32 host software", layout, location = (0,0), size=(1700,900), resizable=True, finalize=True)
window.Maximize()


canvas_elem0 = window['-CANVAS0-']
canvas0 = canvas_elem0.TKCanvas

fig0 = Figure(figsize=(9, 6),dpi=80)
ax0 = fig0.add_subplot(111)
ax0.set_title("Channel 0 - Pressure")
ax0.set_xlabel("Time (s)")
ax0.set_ylabel("Pressure (mBar)")
ax0.grid()
fig_agg0 = draw_figure(canvas0, fig0)


canvas_elem1 = window['-CANVAS1-']
canvas1 = canvas_elem1.TKCanvas

fig1 = Figure(figsize=(9, 6),dpi=80)
ax1 = fig1.add_subplot(111)
ax1.set_title("Channel 1 - Current")
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Current (A)")
ax1.grid()
fig_agg1 = draw_figure(canvas1, fig1)


canvas_elem2 = window['-CANVAS2-']
canvas2 = canvas_elem2.TKCanvas

fig2 = Figure(figsize=(9, 6),dpi=80)
ax2 = fig2.add_subplot(111)
ax2.set_title("Channel 2 - Voltage")
ax2.set_xlabel("Time (s)")
ax2.set_ylabel("Voltage (V)")
ax2.grid()
fig_agg2 = draw_figure(canvas2, fig2)


canvas_elem3 = window['-CANVAS3-']
canvas3 = canvas_elem3.TKCanvas

fig3 = Figure(figsize=(9, 6),dpi=80)
ax3 = fig3.add_subplot(111)
ax3.set_title("Channel 3 - Pressure")
ax3.set_xlabel("Time (s)")
ax3.set_ylabel("Pressure (mBar)")
ax3.grid()
fig_agg3 = draw_figure(canvas3, fig3)

old_t = time.time()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# Run the Event Loop
while True:

    event, values = window.read(timeout=10)

    delta = time.time() - old_t

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Arduino talk

    if Acquisition and delta > (1/readFreq):
        
        if arduino.in_waiting == 0 and Written == False:
            Written = True
            write_data(str(Channel))

        elif arduino.in_waiting > 0:
            Written = False
            measurement = read_data()

            try:
                DATA =  float(measurement)

                if Channel == 0:
                    dataCh0.append(DATA);
                    timeCh0.append(time.time() - initial_t);
                    window["-LOG-"].update(enlaps(initial_t)+" - measured at CH0: "+ str(measurement)+" V. \n" , append=True)
                    graph_update(ax0, timeCh0, dataCh0, fig_agg0, rollSize, "Time (s)", "Pressure (mBar)", "Channel 0 - Pressure", "k")

                elif Channel == 1:
                    dataCh1.append(DATA);
                    timeCh1.append(time.time() - initial_t);
                    window["-LOG-"].update(enlaps(initial_t)+" - measured at CH1: "+ str(measurement)+" V. \n" , append=True)
                    graph_update(ax1, timeCh1, dataCh1, fig_agg1, rollSize, "Time (s)", "Current (A)", "Channel 1 - Current", "r")      

                elif Channel == 2:
                    dataCh2.append(DATA);
                    timeCh2.append(time.time() - initial_t);
                    window["-LOG-"].update(enlaps(initial_t)+" - measured at CH2: "+ str(measurement)+" V. \n" , append=True)
                    graph_update(ax2, timeCh2, dataCh2, fig_agg2, rollSize, "Time (s)", "Voltage (V)", "Channel 2 - Voltage", "b")       

                elif Channel == 3:
                    dataCh3.append(DATA);
                    timeCh3.append(time.time() - initial_t);
                    window["-LOG-"].update(enlaps(initial_t)+" - measured at CH3: "+ str(measurement)+" V. \n" , append=True)
                    graph_update(ax3, timeCh3, dataCh3, fig_agg3, rollSize, "Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "g")                

                # calcP = volts_to_pressure(measurement)

                # data.append(float(calcP))
                # datat.append(time.time() - initial_t)

                # window["-LOG-"].update(enlaps(initial_t)+" - measured: "+ str(measurement)+" V. Calc:" +str('{:0.2e}'.format(calcP))+ " Toor \n" , append=True)
                
                # graph_update(ax, datat, data, fig_agg, rollSize)

            except:
                window["-LOG-"].update(str(measurement)+"\n" , append=True)
            
            old_t = time.time()
            Channel = Channel + 1;
            if Channel >= 4:
                Channel = 0;
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
            arduino.read_all()
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
        window["-LOG-"].update(enlaps(initial_t)+" - Acquisition stopped. ("+str(len(dataCh0))+" points at CH0) \n" , append=True)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the Rolling checkbox
    elif event == "-checkbox-": 
        Rolling  = values["-checkbox-"]
        rollSize = int(values["-rolling-"])
        if not Acquisition:
            graph_update(ax0, timeCh0, dataCh0, fig_agg0, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "k")
            graph_update(ax1, timeCh1, dataCh1, fig_agg1, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "r")
            graph_update(ax2, timeCh2, dataCh2, fig_agg2, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "b")
            graph_update(ax3, timeCh3, dataCh3, fig_agg3, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "g")
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Behaviour of the LogScale checkbox
    elif event == "-checklog-": 
        LogScale  = values["-checklog-"]
        if not Acquisition:
            graph_update(ax0, timeCh0, dataCh0, fig_agg0, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "k")
            graph_update(ax1, timeCh1, dataCh1, fig_agg1, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "r")
            graph_update(ax2, timeCh2, dataCh2, fig_agg2, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "b")
            graph_update(ax3, timeCh3, dataCh3, fig_agg3, rollSize,"Time (s)", "Pressure (mBar)", "Channel 3 - Pressure", "g")

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
        if dataCh0:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH0.txt")
            f = open(filename, "w")
            f.write("data;time\n")
            for (mesItem, timItem) in zip(dataCh0, timeCh0):
                f.write(str(mesItem)+";"+str(timItem)+"\n")
            f.close()
            window["-LOG-"].update(enlaps(initial_t)+" - Data saved to "+ filename +" \n" , append=True)

        if dataCh1:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH1.txt")
            f = open(filename, "w")
            f.write("data;time\n")
            for (mesItem, timItem) in zip(dataCh1, timeCh1):
                f.write(str(mesItem)+";"+str(timItem)+"\n")
            f.close()
            window["-LOG-"].update(enlaps(initial_t)+" - Data saved to "+ filename +" \n" , append=True)

        if dataCh2:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH2.txt")
            f = open(filename, "w")
            f.write("data;time\n")
            for (mesItem, timItem) in zip(dataCh2, timeCh2):
                f.write(str(mesItem)+";"+str(timItem)+"\n")
            f.close()
            window["-LOG-"].update(enlaps(initial_t)+" - Data saved to "+ filename +" \n" , append=True)

        if dataCh3:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH3.txt")
            f = open(filename, "w")
            f.write("data;time\n")
            for (mesItem, timItem) in zip(dataCh3, timeCh3):
                f.write(str(mesItem)+";"+str(timItem)+"\n")
            f.close()
            window["-LOG-"].update(enlaps(initial_t)+" - Data saved to "+ filename +" \n" , append=True)

        else:
            window["-LOG-"].update(enlaps(initial_t)+" - Data is empty. Try after acquiring data!\n" , append=True)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
# Behaviour of the Save graph button
    elif event == "Save graph" and not Acquisition: 
        if dataCh0:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH0.png")
            fig0.savefig(filename, dpi=300)
            window["-LOG-"].update(enlaps(initial_t)+" - Figure saved to "+ filename +" \n" , append=True)

        if dataCh1:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH1.png")
            fig1.savefig(filename, dpi=300)
            window["-LOG-"].update(enlaps(initial_t)+" - Figure saved to "+ filename +" \n" , append=True)

        if dataCh2:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH2.png")
            fig2.savefig(filename, dpi=300)
            window["-LOG-"].update(enlaps(initial_t)+" - Figure saved to "+ filename +" \n" , append=True)

        if dataCh3:
            filename = str(values["-Save data FOLDER-"]+"\\"+str(today.strftime("%b-%d-%Y"))+"_CH3.png")
            fig3.savefig(filename, dpi=300)
            window["-LOG-"].update(enlaps(initial_t)+" - Figure saved to "+ filename +" \n" , append=True)

        else:
            window["-LOG-"].update(enlaps(initial_t)+" - Data is empty. Try after acquiring data!\n" , append=True)



window.close()