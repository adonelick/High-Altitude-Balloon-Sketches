#include <SoftwareSerial.h>

SoftwareSerial mySerial(2,3); // RX, TX

void setup()
{
  Serial.begin(4800);
  mySerial.begin(4800);
}

void loop()
{
  while (Serial.available() > 0)
  {
    mySerial.write(Serial.read());
  }
  
  while (mySerial.available() > 0)
  {
    Serial.write(mySerial.read());
  }
  
}

