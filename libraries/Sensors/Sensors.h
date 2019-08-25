// Written by Andrew Donelick
// 13 December 2015
// adonelick@hmc.edu


#ifndef SENSORS_H
#define SENSORS_H 1

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#include "OneWire.h"
#include "Wire.h"

// These definitions define pin numbers for the 
// analog sensors
#define ANALOG_HUMIDITY A0
#define INTERNAL_TEMP A1
#define HEATER_TEMP A2

// These definitions define the digital sensor pin numbers
#define EXTERNAL_TEMP 4
#define BMP085_ADDRESS 0x77

// Temperature calibration values (in 0.01 degrees C)
#define ANALOG_INTERIOR_OFFSET 200
#define HEATER_OFFSET 400

// Number of readings to take from the analog sensors (for averaging) 
#define NUM_READINGS 5

class Sensors
{

    private:

        // OneWire objects for reading from the digital temp sensors
        OneWire externalTemp_;

        // Calibration values for the BMP085 pressure sensor
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

        // b5 is calculated in bmp085GetTemperature(...), 
        // this variable is also used in bmp085GetPressure(...)
        // so ...Temperature(...) must be called before ...Pressure(...).
        long b5;

        // BMP085 oversampling setting
        unsigned char OSS;


    public:

        Sensors();

        void begin();

        float getAnalogHumidity(int pinNumber);

        float getAnalogTemperature(int pinNumber);

        float getDigitalTemperature(int source);

        short bmp085GetTemperature();

        long bmp085GetPressure();

        float getAltitude();

    private:

        void bmp085Calibration();

        char bmp085Read(unsigned char address);

        int bmp085ReadInt(unsigned char address);

        unsigned int bmp085ReadUT();

        unsigned long bmp085ReadUP();
};

#endif
