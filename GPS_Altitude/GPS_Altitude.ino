#include <SoftwareSerial.h>
#include "TinyGPS++.h"

TinyGPSPlus gps;
SoftwareSerial gpsSerial(10,4); // RX, TX

void setup()
{
  gpsSerial.begin(4800);
  Serial.begin(9600);
}

void loop()
{
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }
  
  
  if (gps.altitude.isUpdated())
  {
    gpsSerial.print('\t');
    gpsSerial.print((unsigned int) gps.altitude.meters());
    gpsSerial.print('\n');
    Serial.println(gps.altitude.meters());
  }
}
