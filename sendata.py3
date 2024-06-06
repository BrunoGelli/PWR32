import influxdb_client, os, time

from influxdb_client import InfluxDBClient, Point, WritePrecision

from influxdb_client.client.write_api import SYNCHRONOUS



from datetime import datetime



import numpy as np



# import serial

# from serial import SerialException

# import time

# import serial.tools.list_ports





#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##                           defining constants



token = os.environ.get("INFLUXDB_TOKEN")

org = "3b837ab9c3c4b3e7"

url = "http://reines.ifi.unicamp.br:8086"

bucket="evaporadora"


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##                           defining a influxDB exporter



client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)



write_api =client.write_api(write_options=SYNCHRONOUS)

   



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##                        read data and export the measurement   



while 1:


    for x in range(100):


        point = (

            Point("Evaporadora")

            .field("Vtest_val", 1.0)

            .field("Itest_val", 1.0)


        )

        write_api.write(bucket=bucket, org="3b837ab9c3c4b3e7", record=point)

        time.sleep(0.5) # separate points by 1 second



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##                                How to query data



# query_api = write_client.query_api()



# query = """from(bucket: "testdata")

# |> range(start: -10m)

# |> filter(fn: (r) => r._measurement == "measurement1")

# |> mean()"""

# tables = query_api.query(query, org="local")



# for table in tables:

#     for record in table.records:

#       print(record)
