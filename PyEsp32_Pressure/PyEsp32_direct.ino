
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
int WifiTime  = 0;

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

void send_data(void * parameters)
{
    for(;;)
    {


        int16_t adc1,adc2,adc3,adc4;

        adc1 = -ads1115_01.readADC_Differential_0_1( );
        adc2 =  ads1115_01.readADC_Differential_2_3();
        adc3 =  ads1115_02.readADC_SingleEnded(1);
        adc4 =  ads1115_02.readADC_Differential_2_3();

        delay(10);

        sensor.clearFields();
        sensor.addField("pressure", ads1115_01.computeVolts(adc1));
        sensor.addField("current",  ads1115_01.computeVolts(adc2));
        sensor.addField("rate",     ads1115_02.computeVolts(adc3));
        sensor.addField("voltage",  ads1115_02.computeVolts(adc4));

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

        request = Serial.readString().toInt();
 

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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() 
{
    // ~~~~~~~~ starting serial and mains starting message ~~~~~~~~
    Serial.begin(115200);
    Serial.setTimeout(1);

    if (Serial.available())
    {
        SerialInterface = true;
    }
    
    Serial.println("");
    Serial.println("~~~~~~~~~~~ starting the setup ~~~~~~~~~~~");
    Serial.print("Running on core: ");
    Serial.println(xPortGetCoreID());
    Serial.println("");


    // ~~~~~~~~ starting the adcs ~~~~~~~~

    Serial.println("Starting the ADCs on 0x48 and 0x49:");

    if(!ads1115_01.begin(0x48))
    {
        Serial.println("  -> failed to start 0x48");
        adc48fail = true;
    } 
    else
    {
        Serial.println("  -> 0x48 correctly started");
        Serial.print("  -> gain set to: "); Serial.println(ads1115_01.getGain()); 
        Serial.print("  -> Max data rate set to: "); Serial.println(ads1115_01.getDataRate()); 

    }

    if(!ads1115_01.begin(0x49))
    {
        Serial.println("  -> failed to start 0x49");
        adc49fail = true;
    }
    else
    {
        Serial.println("  -> 0x49 correctly started");
        Serial.print("  -> gain set to: "); Serial.println(ads1115_02.getGain()); 
        Serial.print("  -> Max data rate set to: "); Serial.println(ads1115_02.getDataRate()); 
    }
    println("");

    // ~~~~~~~~ wifi setup ~~~~~~~~
    Serial.println("Starting wifi connection:");

    WiFi.mode(WIFI_STA);                            // starting the wifi interface as an station (enables connection to an Access point)
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    
    while (wifiMulti.run() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(100);
        WifiTime++;
        if (WifiTime/600 > 2)
        {
            Serial.println("  -> failed to start Wifi");    
            Serial.println("-------> Failed to start wifi. <-------");
            WifiFail = true;
            break;
        }
    }
    if (!WifiFail)
    {
        Serial.println("  -> Connected!");
        Serial.print("  -> Rede: "); Serial.println(WIFI_SSID);
        Serial.print("  -> IP address: "); Serial.println(WiFi.localIP());
        Serial.print("  -> signal strength: "); Serial.println(WiFi.RSSI());

    }
    Serial.println();
  

    // ~~~~~~~~ NTP setup ~~~~~~~~
    if (!WifiFail)
    {
        // Accurate time is necessary for certificate validation and writing in batches to the database
        // I used the NTP servers at the IFGW
        Serial.println("Starting time sync with local NTP:");
        configTime(0,0,"ntp.ifi.unicamp.br");
        setenv("TZ","<-03>3",1);                                //this lines are elvish to me
        tzset();                                                //this lines are elvish to me - understanding and managing timezones is above my paygrade
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo))
        {
          Serial.println("  -> No time available (yet)");
          NtpFail = true;
        }
        else
        {
          Serial.print("    -> Got time sync: "); Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        }
    }
    else
    {
        Serial.println("-------> NTP syncing will not be attempted due to wifi fail. <-------");
        NtpFail = true;
    }

    // ~~~~~~~~ InfluxDB setup ~~~~~~~~
    if (!WifiFail)
    {
        Serial.println("Trying connection to the influxDB:");       // Check server connection
        
        if (client.validateConnection())                            //connecting to influxDB
        {
            Serial.print("  -> Connected to InfluxDB: ");   Serial.println(client.getServerUrl());
        } 
        else 
        {
            Serial.print("  -> InfluxDB connection failed: ");  Serial.println(client.getLastErrorMessage());
            InfluxFail = true;
        }
    }
    else
    {
        Serial.println("-------> InfluxDB connection will not be attempted due to wifi fail. <-------");
        InfluxFail = true;
    }
    Serial.println();


    // ~~~~~~~~ finishing setup ~~~~~~~~
    Serial.print("Creating task at core 0: ");

    if (!WifiFail && !NtpFail && !InfluxFail)
    {
        xTaskCreatePinnedToCore(send_data,"Task1",10000,NULL,100,&Task1,0);
        Serial.println("    -> Network ok. Sending data directly to the InfluxDB. ");
        sensor.addTag("date", "test");
    }
    else 
    {
        xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,100,&Task1,0);
        Serial.println("    -> Network not ok. Sending data over serial. (needs PWR32.py3 to query and plot)");
    }

    Serial.println("~~~~~~~~~~~ finishing the setup ~~~~~~~~~~~");
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() 
{
    delay(1000);
}
