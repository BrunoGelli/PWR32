
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "config.h"

#include <Wire.h>
#include "Adafruit_ADS1X15.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ from influxdb ~~~~~~~~~~~~~~~~~~~~~~~~~

  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"

  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  
  // WiFi AP SSID
  #define WIFI_SSID "ifgw-iot"
  // WiFi password
  #define WIFI_PASSWORD "MSZfsv7Fp@HK0c4v#XW8T.J.t60RySoj6FRLi5haYA,xrAPDR+Lmgt95675"
  
  #define INFLUXDB_URL "http://143.106.128.216:8086"
  #define INFLUXDB_TOKEN "qpx7_LiYtbLXZUE4t4TtL1mySRoc4UywWviqu7YsE5CJI3_dwj8MDmM6S3hcfsNUR_Mnmu8pec-bMJNjN2tdsw=="
  #define INFLUXDB_ORG "3b837ab9c3c4b3e7"
  #define INFLUXDB_BUCKET "evaporadora"
 
  #define INFLUXDB_CLIENT_DEBUG_ENABLE

  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
  
  // Declare Data point
  Point sensor("wifi_test");
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ definitions ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define   AnalogPin     36          // ADC pin
#define   WifiSwitch    22          // Wifi select pin

#define   Verbose       false       // Verbose

#define   WifiSleep     5000        // 3 seconds sleep between wifi pings
#define   AcqBuffer     10000       // 10 seconds with no serial communication

#define   WifiLED       33          // pin for the wifi status led
#define   ledblink      100         // blink period of the led

#define   CorrectLin    0.5328      // 
#define   CorrectAng    0.0036      // 

const char* ssid = "ifgw-iot";
const char* pasw = "MSZfsv7Fp@HK0c4v#XW8T.J.t60RySoj6FRLi5haYA,xrAPDR+Lmgt95675";
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ variable ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool acquisition = false;
bool WifiEnable  = false;
bool WifiInit    = false;
bool flag        = false;

int count     = 0;
int request   = 0;
int measured  = 0;

double DataTask1  = 0;
double DataTask2  = 0;
double CorLinCoef = 0;
double CorAngCoef = 1;

unsigned long time_acquisition = 0;

// AdafruitIO_Feed *counter = io.feed("pressure");

TaskHandle_t Task1;
TaskHandle_t Task2;

Adafruit_ADS1115 ads1115_01;
Adafruit_ADS1115 ads1115_02;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Core1StartMessage(bool Verb)
{
    if (Verb)
    {
        Serial.println("");
        Serial.println("~~~~~~~~~~~ starting the core 0 test ~~~~~~~~~~~");

        Serial.print("Running on core: ");
        Serial.println(xPortGetCoreID());

        Serial.print("Connecting to IO: ");
    }
}

void Core1StopMessage(bool Verb, double value)
{
    if (Verb)
    {
        Serial.println(". Connection was sucessfull.");

        Serial.print("sending data -> ");
        Serial.println(value);

        Serial.println("~~~~~~~~~~~ finishing the core 0 test ~~~~~~~~~~~");
    }
}

int Core1Sampling()
{
    if (!acquisition)
    {
        return analogRead(AnalogPin);
    }
    else
        return DataTask1;
}

void Core0Message(bool Verb, double value)
{
    if (Verb)
    {
        Serial.print("I'm doing something at core ");
        Serial.println(xPortGetCoreID());
        Serial.print(value);
        Serial.println(" adc channels.");
    }
    else
    {
        Serial.print(int(value));
        Serial.print("\n");
    }
}


void acquisition_status()
{
    unsigned long int current_time = millis();

    if (time_acquisition - current_time > AcqBuffer)
    {
        acquisition = false;
    }

}

double calibrate(int value)
{
    return value * CorrectAng + CorrectLin;
}

void check_connection()
{
    // int counter = 0;
    // while(io.status() < AIO_CONNECTED)
    // {
    //     Serial.print(".");
    //     digitalWrite(WifiLED, LOW); 
    //     delay(250);
    //     digitalWrite(WifiLED, HIGH);
    //     delay(250);
    //     counter++;
    //     if (counter>30)
    //     {
    //         Serial.print("Timeout! Restarting the board...");
    //         ESP.restart();
    //     }
    // }

}

void send_data(void * parameters)
{
    for(;;)
    {


        int16_t adc1,adc2,adc3,adc4;

        adc1 = -ads1115_01.readADC_Differential_0_1( );
        adc2 = ads1115_01.readADC_Differential_2_3();
        adc3 = ads1115_02.readADC_SingleEnded(1);
        adc4 = ads1115_02.readADC_Differential_2_3();

        delay(10);

        sensor.clearFields();
        sensor.addField("pressure", ads1115_01.computeVolts(adc1));
        sensor.addField("current",  ads1115_01.computeVolts(adc2));
        sensor.addField("rate",  ads1115_02.computeVolts(adc3));
        sensor.addField("voltage",     ads1115_02.computeVolts(adc4));

        // Print what are we exactly writing
        Serial.print("Writing: ");
        Serial.println(sensor.toLineProtocol());

        // Check WiFi connection and reconnect if needed
        if (wifiMulti.run() != WL_CONNECTED) {
          Serial.println("Wifi connection lost");
        }

        // Write point
        if (!client.writePoint(sensor)) {
          Serial.print("InfluxDB write failed: ");
          Serial.println(client.getLastErrorMessage());
        }
      delay(1000);
    }


}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ Task1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Task1code(void * parameters)
{
    for(;;)
    {
        while(!Serial.available())
        {
            delay(10);
        }

        request     = Serial.readString().toInt();
 

        int16_t adc;

        switch (request) 
        {
            case 0:
                // adc = ads1115_01.readADC_SingleEnded(0);
                adc=-ads1115_01.readADC_Differential_0_1( );
                Serial.print(ads1115_01.computeVolts(adc), 4);
                break;

            case 1:
                adc = ads1115_01.readADC_Differential_2_3();
                Serial.print(ads1115_01.computeVolts(adc), 4);
                break;

            case 2:
                adc = ads1115_02.readADC_SingleEnded(1);
                Serial.print(ads1115_02.computeVolts(adc), 4);
                break;

            case 3:
                adc = ads1115_02.readADC_Differential_2_3();
                Serial.print(ads1115_02.computeVolts(adc), 4);
                break;

            // case 4:
            //     adc = ads1115_02.readADC_SingleEnded(0);
            //     Serial.print(ads1115_02.computeVolts(adc), 4);
            //     break;

            // case 5:
            //     adc = ads1115_02.readADC_SingleEnded(1);
            //     Serial.print(ads1115_02.computeVolts(adc), 4);
            //     break;

            // case 6:
            //     adc = ads1115_02.readADC_SingleEnded(2);
            //     Serial.print(ads1115_02.computeVolts(adc), 4);
            //     break;

            // case 7:
            //     adc = ads1115_02.readADC_SingleEnded(3);
            //     Serial.print(ads1115_02.computeVolts(adc), 4);
            //     break;

            default:
                adc = 666;
                Serial.print("Error in the request: ");
                Serial.print(request);
                break;
        }
                
        Serial.print("\n");

        delay(10);
    }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() 
{
    pinMode(WifiLED, OUTPUT);
    digitalWrite(WifiLED, LOW); 


    pinMode(WifiSwitch, INPUT);
    delay(10);
    WifiEnable = false;// digitalRead(WifiSwitch);

    Serial.begin(115200);
    Serial.setTimeout(1);

    ads1115_01.begin(0x49);
    ads1115_02.begin(0x48);


    Serial.println("");
    Serial.println("~~~~~~~~~~~ starting the setup ~~~~~~~~~~~");
    Serial.print("Running on core: ");
    Serial.println(xPortGetCoreID());

    // Setup wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();
    Serial.print("  Connected!");
    Serial.println();
  
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    Serial.println("Connecting to NTP:");
    configTime(0,0,"ntp.ifi.unicamp.br");
    setenv("TZ","<-03>3",1);
    tzset();
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
      Serial.println("  No time available (yet)");
    }
    else
    {
      Serial.print("  Got time sync: ");
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }

    // Check server connection
    Serial.println("Trying connection to the influxDB:");
    
    //connecting to influxDB
    if (client.validateConnection()) 
    {
      Serial.print("  Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
      flag = true;
    } 
    else 
    {
      Serial.print("  InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    Serial.println();
    Serial.println("~~~~~~~~~~~ finishing the setup ~~~~~~~~~~~");

    Serial.print("Creating the core 0 task: ");
    if (flag)
    {
        xTaskCreatePinnedToCore(send_data,"Task1",10000,NULL,100,&Task1,0);
        Serial.println("sending directly to the InfluxDB.");
        sensor.addTag("date", "test");

    }
    else 
    {
      xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,100,&Task1,0);
      Serial.println("sending data over serial.");
    }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() 
{
    WifiEnable = false;//digitalRead(WifiSwitch);
    
    if (WifiInit == true && WifiEnable == false)
    {
        // Serial.println("Disconnecting from Adafruit IO.");
        // WifiInit == false;
        // digitalWrite(WifiLED,LOW);
        // Serial.println("done.");
    }

    if (WifiEnable)
    {
        // if (!WifiInit)
        // {   
        //     Serial.print("Connecting to Adafruit IO... ");
        //     io.connect();
        // }
        acquisition_status();
        
        check_connection();
        
        DataTask2 = Core1Sampling();
        DataTask2 = calibrate(DataTask2);
        DataTask2 = pow(10, (DataTask2/0.6)-12);

        Core1StartMessage(Verbose);

        // io.run(1,true);
        // counter->save(DataTask2);

        digitalWrite(WifiLED, LOW); 
        delay(ledblink);
        digitalWrite(WifiLED, HIGH);

        Core1StopMessage(Verbose, DataTask2);
    }

    delay(WifiSleep);
}
