
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ includes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "config.h"

#include <Wire.h>
#include "Adafruit_ADS1X15.h"

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ variable ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool acquisition = false;
bool WifiEnable  = false;
bool WifiInit    = false;

int count     = 0;
int request   = 0;
int measured  = 0;

double DataTask1  = 0;
double DataTask2  = 0;
double CorLinCoef = 0;
double CorAngCoef = 1;

unsigned long time_acquisition = 0;

AdafruitIO_Feed *counter = io.feed("pressure");

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
    int counter = 0;
    while(io.status() < AIO_CONNECTED)
    {
        Serial.print(".");
        digitalWrite(WifiLED, LOW); 
        delay(250);
        digitalWrite(WifiLED, HIGH);
        delay(250);
        counter++;
        if (counter>30)
        {
            Serial.print("Timeout! Restarting the board...");
            ESP.restart();
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

        request     = Serial.readString().toInt();
 

        int16_t adc;

        switch (request) 
        {
            case 0:
                adc = ads1115_01.readADC_SingleEnded(0);
                break;

            case 1:
                adc = ads1115_01.readADC_SingleEnded(1);
                break;

            case 2:
                adc = ads1115_01.readADC_SingleEnded(2);
                break;

            case 3:
                adc = ads1115_01.readADC_SingleEnded(3);
                break;

            case 4:
                adc = ads1115_02.readADC_SingleEnded(0);
                break;

            case 5:
                adc = ads1115_02.readADC_SingleEnded(1);
                break;

            case 6:
                adc = ads1115_02.readADC_SingleEnded(2);
                break;

            case 7:
                adc = ads1115_02.readADC_SingleEnded(3);
                break;

            default:
                adc = 666;
                break;
        }
        
        delay(10);

        if (adc == 666)
        {
            Serial.print("Error in the request: ");
            Serial.print(request);
        }
        else
            Serial.print(ads1115_01.computeVolts(adc), 4);
        
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

    ads1115_01.begin(0x48);
    ads1115_02.begin(0x49);


    Serial.println("");
    Serial.println("~~~~~~~~~~~ starting the setup ~~~~~~~~~~~");
    Serial.print("Running on core: ");
    Serial.println(xPortGetCoreID());

    if(WifiEnable)
    {
        WifiInit = true;
        Serial.print("Connecting to Adafruit IO... ");
        io.connect();
        check_connection();
        Serial.println("done.");
        Serial.println(io.statusText());
    }
    else
        Serial.println("Wifi data logging is off");

    
    Serial.println("~~~~~~~~~~~ finishing the setup ~~~~~~~~~~~");

    Serial.print("Creating the core 0 task: ");
    xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,100,&Task1,0);
    Serial.println("done.");
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~ LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() 
{
    WifiEnable = false;//digitalRead(WifiSwitch);

    if (WifiInit == true && WifiEnable == false)
    {
        Serial.println("Disconnecting from Adafruit IO.");
        WifiInit == false;
        digitalWrite(WifiLED,LOW);
        Serial.println("done.");
    }

    if (WifiEnable)
    {
        if (!WifiInit)
        {   
            Serial.print("Connecting to Adafruit IO... ");
            io.connect();
        }
        acquisition_status();
        
        check_connection();
        
        DataTask2 = Core1Sampling();
        DataTask2 = calibrate(DataTask2);
        DataTask2 = pow(10, (DataTask2/0.6)-12);

        Core1StartMessage(Verbose);

        io.run(1,true);
        counter->save(DataTask2);

        digitalWrite(WifiLED, LOW); 
        delay(ledblink);
        digitalWrite(WifiLED, HIGH);

        Core1StopMessage(Verbose, DataTask2);
    }

    delay(WifiSleep);
}
