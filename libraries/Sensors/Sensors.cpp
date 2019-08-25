// Written by Andrew Donelick
// 13 December 2015
// adonelick@hmc.edu

#include "Sensors.h"


Sensors::Sensors()
    : externalTemp_(EXTERNAL_TEMP)
{
    OSS = 3;
}


void Sensors::begin()
{
    Wire.begin();
    bmp085Calibration();
}

float getAltitude()
{
    return 0.0;
}


/*
 * Read the analog voltage from the specified 
 * pin and return the corresponding temperature.
 * For use with TMP36 temperature sensors
 */
float Sensors::getAnalogTemperature(int pinNumber)
{
    unsigned int total = 0;
    for (int i = 0; i < NUM_READINGS; ++i)
    {
        total += analogRead(pinNumber);
    }

    float reading = (total / NUM_READINGS) * (5000.0 / 1024.0);
    return (reading - 500.0) / 10.0;
}


float Sensors::getAnalogHumidity(int pinNumber)
{
    unsigned int total = 0;
    for (int i = 0; i < NUM_READINGS; ++i)
    {
        total += analogRead(pinNumber);
    }
    return ((total / NUM_READINGS) - 196.2) / 6.3;
}


/*
 * Get the temperature from the digital temperature
 * sensor. Meant to be used with DS18S20 temperature sensor.
 * Returns the temperature from one DS18S20 in DEG Celsius
 */
float Sensors::getDigitalTemperature(int source)
{

    // This isn't very interesting right now, but if more digital
    // sensors were added, choosing between them would go here.

    OneWire ds(externalTemp_);
    if (source != EXTERNAL_TEMP)
    {
        // Do nothing
    }

    byte data[12];
    byte addr[8];

    if ( !ds.search(addr)) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return -1000;
    }

    if ( OneWire::crc8( addr, 7) != addr[7]) {
        // Serial.println("CRC is not valid!");
        return -1000;
    }

    if ( addr[0] != 0x10 && addr[0] != 0x28) {
        // Serial.print("Device is not recognized");
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


// The following functions deal with interfacing with the 
// BMP_085 sensor

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void Sensors::bmp085Calibration()
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
short Sensors::bmp085GetTemperature()
{
    unsigned int ut = bmp085ReadUT();
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
long Sensors::bmp085GetPressure()
{
    unsigned long up = bmp085ReadUP();
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
char Sensors::bmp085Read(unsigned char address)
{
    unsigned char data;

    Wire.beginTransmission(BMP085_ADDRESS);
    Wire.write(address);
    Wire.endTransmission();

    Wire.requestFrom(BMP085_ADDRESS, 1);

    unsigned long startTime = millis();
    while(!Wire.available())
    ;

    if (!Wire.available())
    {
        return 0;
    } else {
        return Wire.read();
    }
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int Sensors::bmp085ReadInt(unsigned char address)
{
    unsigned char msb, lsb;

    Wire.beginTransmission(BMP085_ADDRESS);
    Wire.write(address);
    Wire.endTransmission();

    Wire.requestFrom(BMP085_ADDRESS, 2);
    unsigned long startTime = millis();
    while(Wire.available() < 2)
    ;
    msb = Wire.read();
    lsb = Wire.read();

    return (int) msb<<8 | lsb;
}


// Read the uncompensated temperature value
unsigned int Sensors::bmp085ReadUT()
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
unsigned long Sensors::bmp085ReadUP()
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
