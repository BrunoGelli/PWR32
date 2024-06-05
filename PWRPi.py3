from datetime import datetime
from datetime import date
from time import sleep

import numpy as np

import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# declaring some important variables


i2c = busio.I2C(board.SCL, board.SDA)
ads = ADS.ADS1115(i2c, data_rate = 8, address = 0x49)

dataCh0 = []
dataCh1 = []
dataCh2 = []
dataCh3 = []
timeCh0 = []
timeCh1 = []
timeCh2 = []
timeCh3 = []

initial_t   = time.time()
today       = date.today()


tooltip_logo    = 'Developed by Bruno Gelli, Aug 2022.\nMore information at github.com/BrunoGelli/PWR32'

# token = os.environ.get("INFLUXDB_TOKEN")
# org = "3b837ab9c3c4b3e7"
# url = "http://reines.ifi.unicamp.br:8086"
# bucket="evaporadora"

# client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)
# write_api =client.write_api(write_options=SYNCHRONOUS)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
#functions




# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
# layout
ads.gain = 16
print("Here we will list the properties of the ADC")
print(" -> The possible data rates are: " + str(ads.rates))
print("     This device is currently set to: " + str(ads.data_rate) + " samples per second.")
print(" -> The dinamic range of this device is: " + str(ads.bits) + " bits.")
print(" -> The possible gains of this device are: " + str(ads.gains))
print("     This device is currently set to: " + str(ads.gain) + "x.")


while True:
    chan = AnalogIn(ads, ADS.P0, ADS.P1)
    print(chan.value, chan.voltage)
    pass