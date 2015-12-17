// Written by: Andrew Donelick
// 13 April 2013

// Runs the flight computer
// Records:
//    temperature inside and outside the module, 
//    baramoetric pressure,
//    humidity

// Also,this code controls the heater for the module

#include <Wire.h>
#include <SdFat.h>
#include <SdFatUtil.h> 

#define BMP085_ADDRESS 0x77 // I2C address of BMP085
#define LOG_INTERVAL     2  // Seconds between sensor readings
#define SD_CHIP_SELECT   8  // SD chip select pin
#define USE_DS1307       0  // set nonzero to use DS1307 RTC
#define ADC_DELAY       10  // ADC delay for high impedence sensors

const int humidityPin = A3;
const int outerTempPin = A2;
const int innerTempPin = A1;
const int logPin = 7;
const int logDisplayPin = 6;
const int heaterPin = 4;

const unsigned char OSS = 0;  // Oversampling Setting from BMP085

// Calibration values for the pressure sensor
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 

float innerTempBMP;
float innerTempTMP;
long pressure;
float humidity;
float outerTemp;

long flightTime;
long startTimer;

int restart = 0;
int dataLog = 1;
boolean fileClosed = false;
int heaterOn = 1;

// create a new file in root, the current working directory
char name[] = "FLIGHT00.CSV";

// file system object
SdFat sd;
// text file for logging
ofstream logfile;
// Serial print stream
ArduinoOutStream cout(Serial);

// buffer to format data - makes it eaiser to echo to Serial
char buf[100];

// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))

// Use these for altitude conversions
//const float p0 = 101325;     // Pressure at sea level (Pa)
//float altitude;

void setup()
{
  Serial.begin(9600);
  pinMode(logPin, INPUT);
  pinMode(logDisplayPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);
  Wire.begin();
  
  // Get the calibration from the BMP085 
  bmp085Calibration();
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  if (!sd.begin(SD_CHIP_SELECT, SPI_HALF_SPEED)) sd.initErrorHalt();

  for (uint8_t i = 0; i < 100; i++) 
  {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    
    if (sd.exists(name))
      continue;
      
    logfile.open(name);
    break;
  }
  
  // The file did not open!
  if (!logfile.is_open())
    error("file.open");
    
  // format header in buffer
  obufstream SDout(buf, sizeof(buf));
  
  SDout << pstr("Seconds,");
  SDout << pstr("Pressure,");
  SDout << pstr("Inner Temp BMP,");
  SDout << pstr("Inner Temp TMP,");
  SDout << pstr("Outer Temp,");
  SDout << pstr("Humidity");
  SDout << pstr("Heater");
  
  logfile << buf << endl;
  
  // start the timer for logging data
  startTimer = millis();
}

void loop()
{
  
  // innerTemp: temperature from the bmp085 sensor (inside)
  // presssure: atmospheric pressure from the bmp085
  // humidity: humidity from the hih-4030
  
  innerTempBMP = float(bmp085GetTemperature(bmp085ReadUT())) / 10.0;
  
  // Control the heater based on internal temperature
  if (innerTempBMP < 0.0) {
    digitalWrite(heaterPin, HIGH);
    heaterOn = 1;
  } else if (innerTempBMP > 5.0) {
    digitalWrite(heaterPin, LOW);
    heaterOn = 0;
  }

  dataLog = digitalRead(logPin);
 
 if ((dataLog) && !(fileClosed))
 {
   // read the data from the sensors
    digitalWrite(logDisplayPin, HIGH);
    innerTempBMP = float(bmp085GetTemperature(bmp085ReadUT())) / 10.0;
    pressure = bmp085GetPressure(bmp085ReadUP());
    // altitude = (float)44330 * (1 - pow(((float) pressure/p0), 0.190295));
    humidity = analogRead(humidityPin);
    humidity = (humidity - 196.2) / 6.3;
    
    outerTemp = analogRead(outerTempPin) * (5000.0 / 1024.0);
    outerTemp = (outerTemp - 500.0) / 10.0;
    
    innerTempTMP = analogRead(innerTempPin) * (5000.0 / 1024.0);
    innerTempTMP = (innerTempTMP - 500.0) / 10.0;
    
    // use buffer stream to format line
    obufstream SDout(buf, sizeof(buf));
    
    flightTime = (millis() - startTimer) / 1000;
    
    // Log the data to the buffer
    SDout << flightTime << ',';
    SDout << pressure << ',';
    SDout << innerTempBMP << ',';
    SDout << innerTempTMP << ',';
    SDout << outerTemp << ',';
    SDout << humidity << ',';
    SDout << heaterOn << endl;
    
    // stuff was written to the card, blink the LED
    digitalWrite(logDisplayPin, LOW);
    delay(40);
    digitalWrite(logDisplayPin, HIGH);
    delay(40);
    digitalWrite(logDisplayPin, LOW);
    delay(40);
    digitalWrite(logDisplayPin, HIGH);

    // log data and flush to SD
    logfile << buf << flush;
  }
  else if ((!dataLog) && (!fileClosed))
  {
    logfile.close();
    digitalWrite(logDisplayPin, LOW);
    fileClosed = true;
  }
  
  delay(LOG_INTERVAL*1000);
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;
    
  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}


