// Written by Andrew Donelick
// Runs the cold testing chamber

#include <OneWire.h>

// DS18S20 Signal pin on digital 2
int DS18S20_Pin = 3;

// Temperature chip i/o
OneWire ds(DS18S20_Pin); // on digital pin 2

void setup ()
{
  Serial.begin(9600);
  delay(500);
  
  Serial.write(254);
  Serial.write(128);
  Serial.write("Temperature:");
}

void loop ()
{
  float temperature = getTemperature();
  
  if (digitalRead(2))
  {
    temperature = convertToFahrenheit(temperature);
  }
  
  // Move the cursor to the beginning of the second line
  Serial.write(254);
  Serial.write(192);
  Serial.print(temperature);
  
  if (digitalRead(2))
  {
    Serial.print(" F     ");
  } else {
    Serial.print(" C     ");
  }
    
  
  delay(1000);
  
}

// Convert the temperature from Celcius
// to Fahrenheit
float convertToFahrenheit(float temperature)
{
  return (temperature * (9.0/5.0)) + 32.0;
}

// Reads the temperature from the sensor
float getTemperature ()
{
 //returns the temperature from one DS18S20 in DEG Celsius

 byte data[12];
 byte addr[8];

 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
   Serial.println("CRC is not valid!");
   return -1000;
 }

 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   Serial.print("Device is not recognized");
   return -1000;
 }

 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end

 byte present = ds.reset();
 ds.select(addr);  
 ds.write(0xBE); // Read Scratchpad

 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];

 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;
 
 return TemperatureSum;
 
}
