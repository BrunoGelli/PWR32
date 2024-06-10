
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "config.h"

#include <Wire.h>
#include "Adafruit_ADS1X15.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

    static const unsigned char PROGMEM logo_bmp[] =
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x01,
0xc1, 0xc0, 0x00, 0x00, 0x3f, 0x80, 0x07, 0xff, 0xff, 0x00, 0xff, 0xff, 0x81, 0xff, 0xff, 0x01,
0xc1, 0xe0, 0x00, 0x00, 0x7f, 0xc0, 0x07, 0xff, 0xff, 0xc1, 0xff, 0xff, 0x83, 0xff, 0xff, 0xc1,
0xc1, 0xe0, 0x00, 0x00, 0x7b, 0xc0, 0x07, 0xff, 0xff, 0xc1, 0xff, 0xff, 0x83, 0xff, 0xff, 0xc1,
0xc1, 0xe0, 0x00, 0x00, 0xf1, 0xe0, 0x07, 0x80, 0x03, 0xc1, 0xf4, 0x00, 0x03, 0xe0, 0x03, 0xc1,
0xc1, 0xe0, 0x00, 0x01, 0xe1, 0xe0, 0x07, 0x80, 0x03, 0xc1, 0xe6, 0x00, 0x03, 0xc0, 0x03, 0xc1,
0xc1, 0xe0, 0x00, 0x01, 0xe0, 0xf0, 0x07, 0x80, 0x03, 0xc1, 0xe2, 0x00, 0x03, 0xc0, 0x03, 0xc1,
0xc1, 0xe0, 0x00, 0x03, 0xc0, 0xf0, 0x07, 0x80, 0x03, 0xc1, 0xf3, 0x00, 0x03, 0xe0, 0x03, 0xc1,
0xc1, 0xe0, 0x00, 0x03, 0xc0, 0x78, 0x07, 0x80, 0x03, 0xc1, 0xff, 0xff, 0x83, 0xff, 0xff, 0xc1,
0xc1, 0xe0, 0x00, 0x07, 0x80, 0x7c, 0x07, 0x80, 0x03, 0xc1, 0xff, 0xff, 0x83, 0xff, 0xff, 0xc1,
0xc1, 0xe0, 0x00, 0x07, 0x80, 0x3c, 0x07, 0x80, 0x03, 0xc1, 0xf0, 0x40, 0x03, 0xe0, 0x00, 0x01,
0xc1, 0xe0, 0x00, 0x0f, 0xff, 0xfe, 0x07, 0x80, 0x03, 0xc1, 0xe0, 0x40, 0x03, 0xc0, 0x00, 0x01,
0xc1, 0xe0, 0x00, 0x0f, 0xff, 0xfe, 0x07, 0x80, 0x03, 0xc1, 0xe0, 0x20, 0x03, 0xc0, 0x00, 0x01,
0xc1, 0xe0, 0x00, 0x1f, 0xff, 0xff, 0x07, 0xc0, 0x03, 0xc1, 0xf0, 0x30, 0x03, 0xc0, 0x00, 0x01,
0xc1, 0xff, 0xfe, 0x3e, 0x00, 0x0f, 0x07, 0xff, 0xff, 0xc1, 0xff, 0xff, 0x83, 0xc0, 0x00, 0x01,
0xc1, 0xff, 0xfe, 0x3c, 0x00, 0x0f, 0x87, 0xff, 0xff, 0xc1, 0xff, 0xff, 0x83, 0xc0, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x40, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x40, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01,
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0x7f, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

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


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ variable ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool SerialInterface = false;
bool WifiEnable  = false;
bool WifiInit    = false;
bool flag        = false;

bool WifiFail    = false;
bool NtpFail     = false;
bool InfluxFail  = false;
bool adc48fail   = false;
bool adc49fail   = false;
bool DisplayFail   = false;

int count     = 0;
int request   = 0;
int WifiTime  = 0;
int timeDelay  = 3;
int measured = 0;
int adc1, adc2, adc3, adc4;

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

void trackMeasurement(int pos, bool displayfail)
{
    display.drawPixel(pos, 60, SSD1306_WHITE);
    display.display();
}

void logo(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - 128 ) / 2,
    (display.height() - 64) / 2,
    logo_bmp, 128, 64, 1);
  display.display();
  delay(1000);
}

void screenTransition(int sec, bool displayfail)
{
    if (displayfail)
    {
        for (int i = 0; i < sec; ++i)
        {
            PrintToAll(".", 1, true, false, false);
            delay(1000);  
        }
    }

}

void PrintToAll(const String& thisIsAString, int size, bool displayfail, bool clear, bool skipline)
{
    if(displayfail)
    {
        if (clear)
        {
            display.clearDisplay();
            display.setCursor(0, 0);
        }
        display.setTextSize(size);                
        display.setTextColor(SSD1306_WHITE);
        if (skipline)
        {
            display.println(thisIsAString);
        }
        else
        {
            display.print(thisIsAString);
        }
        display.display();       
    }

    if (skipline)
    {
        Serial.println(thisIsAString);
    }
    else
    {
        Serial.print(thisIsAString);
    }
    
}

void send_data(void * parameters)
{
    for(;;)
    {   
        delay(1000);
        
        PrintToAll("Measuring online:", 1, !DisplayFail, true, true);
        
        measured++;
        trackMeasurement(measured, !DisplayFail);
        if (measured > 127)
        {
            measured = 0;
        }
        
        sensor.clearFields();
        
        // Check WiFi connection and reconnect if needed
        if (wifiMulti.run() != WL_CONNECTED) {
            PrintToAll(" -> Wifi lost!", 1, !DisplayFail, false, true);
        }
        else
        {
            PrintToAll(" -> Wifi ok!", 1, !DisplayFail, false, true);
        }

        if (!adc48fail)
        {
            adc1 = -ads1115_01.readADC_Differential_0_1();
            adc2 =  ads1115_01.readADC_Differential_2_3();
            sensor.addField("pressure", ads1115_01.computeVolts(adc1));
            sensor.addField("current",  ads1115_01.computeVolts(adc2));
            PrintToAll("adc1="+String(ads1115_01.computeVolts(adc1)),1,!DisplayFail, false, false);
            PrintToAll("|adc2="+String(ads1115_01.computeVolts(adc1)),1,!DisplayFail, false, false);
        }
        if (!adc49fail)
        {
            adc3 =  ads1115_02.readADC_SingleEnded(1);
            adc4 =  ads1115_02.readADC_Differential_2_3();
            sensor.addField("rate",     ads1115_02.computeVolts(adc3));
            sensor.addField("voltage",  ads1115_02.computeVolts(adc4));
            PrintToAll("|adc3="+String(ads1115_01.computeVolts(adc1)),1,!DisplayFail, false, false);
            PrintToAll("|adc4="+String(ads1115_01.computeVolts(adc1)),1,!DisplayFail, false, false);
        }
        
        PrintToAll("",1,!DisplayFail, false, true);

        if(SerialInterface)
        {
            Serial.println(sensor.toLineProtocol());
        }

        // Write point
        if (!client.writePoint(sensor)) 
        {
            // PrintToAll("InfluxDB failed!", 1, !DisplayFail, false, true);
            PrintToAll(String(client.getLastErrorMessage()), 1, !DisplayFail, false, true);
        }
        else
        {
            PrintToAll("InfluxDB ok!", 1, !DisplayFail, false, true);
        }


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

    // ~~~~~~~~ starting the screen ~~~~~~~~

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))     // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    {
        Serial.println("  -> SSD1306 allocation failed");
        DisplayFail = true;
    }
    else
    {
        logo();
        delay(2000);
        
        PrintToAll("PWR32",3, !DisplayFail, true, true); 
        PrintToAll("github.com/BrunoGelli/PWR32", 1, !DisplayFail, false, true); 
        delay(2000);



        PrintToAll("Serial:", 2, !DisplayFail, true, true); 
        if (SerialInterface)
        {
            PrintToAll(" -> ok", 1, !DisplayFail, false, true);  
        }
        else
        {
            PrintToAll(" -> not connected", 1, !DisplayFail, false, true);  
        }
       
        PrintToAll("", 1, !DisplayFail, false, true);  

        PrintToAll("Screen:", 2, !DisplayFail, false, true); 
        PrintToAll(" -> ok", 1, !DisplayFail, false, true);  
    }
    screenTransition(timeDelay, !DisplayFail);
    Serial.println();

    // ~~~~~~~~ starting the adcs ~~~~~~~~

    PrintToAll("ADCs:", 2, !DisplayFail, true, true); 

    if(!ads1115_01.begin(0x48))
    {
        PrintToAll("  -> 0x48 failed", 1, !DisplayFail, false, true);
        adc48fail = true;
    } 
    else
    {
        PrintToAll("  -> 0x48 ok!", 1, !DisplayFail, false, true);
        PrintToAll("    -> gain: " + String(ads1115_01.getGain()), 1, !DisplayFail, false, true); 
        PrintToAll("    -> data rate: " + String(ads1115_01.getDataRate()), 1, !DisplayFail, false, true); 
    }

    if(!ads1115_02.begin(0x49))
    {
        PrintToAll("  -> 0x49 failed", 1, !DisplayFail, false, true);
        adc49fail = true;
    }
    else
    {
        PrintToAll("  -> 0x49 ok!", 1, !DisplayFail, false, true);
        PrintToAll("    -> gain: " + String(ads1115_02.getGain()), 1, !DisplayFail, false, true); 
        PrintToAll("    -> data rate: " + String(ads1115_02.getDataRate()), 1, !DisplayFail, false, true); 
    }
    screenTransition(timeDelay, !DisplayFail);
    Serial.println();

    // ~~~~~~~~ wifi setup ~~~~~~~~
    PrintToAll("WiFi:", 2, !DisplayFail, true, true); 

    WiFi.mode(WIFI_STA);                            // starting the wifi interface as an station (enables connection to an Access point)
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    
    while (wifiMulti.run() != WL_CONNECTED) 
    {
        PrintToAll(".", 1, !DisplayFail, false, false);       
        delay(100);
        WifiTime++;
        if (WifiTime > 3)
        {
            PrintToAll("", 1, !DisplayFail, false, true);    
            PrintToAll("  -> Wifi failed", 1, !DisplayFail, false, true);    
            Serial.println("-------> Failed to start wifi. <-------");
            WifiFail = true;
            break;
        }
    }
    if (!WifiFail)
    {
        PrintToAll("-> Connected!", 1, !DisplayFail, false, true);
        PrintToAll("  -> " + String(WIFI_SSID),      1, !DisplayFail, false, true); 
        PrintToAll("  -> " + WiFi.localIP().toString(), 1, !DisplayFail, false, true);       
        PrintToAll("  -> " + String(WiFi.RSSI()) + " dB",    1, !DisplayFail, false, true);   
    
        screenTransition(timeDelay, !DisplayFail);
        Serial.println();
    }

  

    // ~~~~~~~~ NTP setup ~~~~~~~~
    if (!WifiFail)
    {
        // Accurate time is necessary for certificate validation and writing in batches to the database
        // I used the NTP servers at the IFGW
        PrintToAll("NTP sync:", 2, !DisplayFail, true, true); 
        configTime(0,0,"pool.ntp.org");
        // configTime(0,0,"ntp.ifi.unicamp.br");
        
        setenv("TZ","<-03>3",1);                                //this lines are elvish to me
        tzset();                                                //this lines are elvish to me - understanding and managing timezones is above my paygrade
        
        struct tm timeinfo;

        if(!getLocalTime(&timeinfo))
        {
            PrintToAll("    -> time sync fail", 1, !DisplayFail, false, true);
            NtpFail = true;
        }
        else
        {
            PrintToAll("    ", 1, !DisplayFail, false, true);
            PrintToAll("  -> ok!", 1, !DisplayFail, false, true);

            char timeStringBuff[50]; //50 chars should be enough
            strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);

            PrintToAll(String(timeStringBuff), 1, !DisplayFail, false, true);

          // Serial.print("    -> Got time sync: "); Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        }

        screenTransition(timeDelay, !DisplayFail);
        Serial.println();
    }
    else
    {
        PrintToAll("", 1, !DisplayFail, false, true);
        PrintToAll("NTP skip - wifi fail", 1, !DisplayFail, false, true);
        // Serial.println("-------> NTP syncing will not be attempted due to wifi fail. <-------");
        NtpFail = true;
    }


    //~~~~~~~~ InfluxDB setup ~~~~~~~~
    if (!WifiFail)
    {
        PrintToAll("InfluxDB:", 2, !DisplayFail, true, true);
        
        if (client.validateConnection())                            //connecting to influxDB
        {
            PrintToAll("-> Connected!", 1, !DisplayFail, false, true);
            PrintToAll("-> " + String(client.getServerUrl()), 1, !DisplayFail, false, true);
        } 
        else 
        {
            PrintToAll("-> Failed!", 1, !DisplayFail, false, true);
            PrintToAll("-> " + String(client.getLastErrorMessage()), 1, !DisplayFail, false, true);
            InfluxFail = true;
        }

    }
    else
    {
        PrintToAll("", 1, !DisplayFail, false, true);
        PrintToAll("InfluxDB skip - wifi fail", 1, !DisplayFail, false, true);
        // Serial.println("-------> InfluxDB connection will not be attempted due to wifi fail. <-------");
        InfluxFail = true;
    }

    screenTransition(timeDelay, !DisplayFail);
    Serial.println();

    // ~~~~~~~~ finishing setup ~~~~~~~~
    PrintToAll("Finishing:", 2, !DisplayFail, true, true);
    PrintToAll("Start task at core 0: ", 1, !DisplayFail, false, true);

    if (!WifiFail && !NtpFail && !InfluxFail)
    {
        PrintToAll("-> Network ok! Sending to InfluxDB.", 1, !DisplayFail, false, true);
    }
    else 
    {
        PrintToAll("-> Network fail!", 1, !DisplayFail, false, true);
        PrintToAll("-> Sending to Serial.", 1, !DisplayFail, false, true);
        PrintToAll("-> Use PWR32.py3", 1, !DisplayFail, false, true);
    }
    
    PrintToAll("~~~~ setup done ~~~~", 1, !DisplayFail, false, true);
    screenTransition(timeDelay, !DisplayFail);
    Serial.println();
    
    if (!WifiFail && !NtpFail && !InfluxFail)
    {
        xTaskCreatePinnedToCore(send_data,"Task1",10000,NULL,100,&Task1,0);
        sensor.addTag("date", "test");
    }
    else 
    {
        xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,100,&Task1,0);
    }

}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() 
{
    delay(1000);
}
