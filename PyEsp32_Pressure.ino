double mean = 0;
double entry = 0;
int analogValue =0;


void setup() {
  // put your setup code here, to run once:
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
}

void loop() {

  while (!Serial.available());
  entry = Serial.readString().toDouble();
  for(int i=0; i<20; i++)
  {
    mean = mean + analogRead(36);
    //analogValue = analogRead(36);
  }

  mean = mean/20;
  
  // print out the values you read:
  Serial.print(entry);
  Serial.print(" ");
  Serial.print(mean);
  Serial.println();
  
  delay(100);  // delay in between reads for clear read from serial
}
