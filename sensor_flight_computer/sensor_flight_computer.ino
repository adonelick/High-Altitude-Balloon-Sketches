#include <OneWire.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SparkFunLSM9DS1.h>
#include <SparkFunMPL3115A2.h>
#include <SparkFun_HIH4030.h>
#include <DataFile.h>
#include "TinyGPS++.h"

/* Calibration constants */
#define VOLTAGE_ADJUSTMENT 1.046
#define RATE_X -0.1368
#define RATE_Y -2.423
#define RATE_Z 0.03344

/* Digital pin definitions */
#define TRANSMISSION_PIN 3
#define TEMPERATURE_PIN_EXTERIOR 6
#define TEMPERATURE_PIN_INTERIOR 7

/* Analog pin definitions */
#define HUMIDITY_PIN A5
#define VOLTAGE_PIN A8

#define DATA_LENGTH 1000
#define LATITUDE_LENGTH 50
#define LONGITUDE_LENGTH 50

/* Communication packet variables */
byte data[DATA_LENGTH];
unsigned int numBytes;

/* Humidity related variables */
HIH4030 humiditySensor(HUMIDITY_PIN, 5);
float humidity;

/* Pressure related variables */
MPL3115A2 pressureSensor;
float pressure;

/* Battery voltage related variables */
float batteryVoltage;

/* Temperature related variables */
OneWire ds_exterior(TEMPERATURE_PIN_EXTERIOR);
OneWire ds_interior(TEMPERATURE_PIN_INTERIOR);
float interiorTemperature1;
float interiorTemperature2;
float exteriorTemperature;

/* GPS related variables */
#define GPS_WARNING_AGE 10000
TinyGPSPlus gps;
unsigned long gpsTime;
unsigned long gpsDate;
char latitude[LATITUDE_LENGTH];
char longitude[LONGITUDE_LENGTH];
unsigned long rawAltitude;
float altitude;
unsigned int rawLatitudeDegrees;
unsigned int rawLongitudeDegrees;
unsigned long rawLatitudeBillionths;
unsigned long rawLongitudeBillionths;
byte rawLatitudeSign;
byte rawLongitudeSign;
boolean gpsValidity;
unsigned long gpsSentences;
unsigned long failedSentences;
unsigned int numSatellites;
float payloadSpeed;

/* Ascent rate variables */
float ascentRate;
float previousAltitude = 0;
unsigned long previousAltitudeTime = 0;
unsigned long altitudeTime = 0;
unsigned int ascentRateIndex = 0;
#define ASCENT_RATE_VALUES 20
float ascentRateHistory[ASCENT_RATE_VALUES];

/* IMU related variables */
#define LSM9DS1_M    0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW
#define DECLINATION -14.32
LSM9DS1 imu;
float ax;
float ay;
float az;
float gx;
float gy;
float gz;
float mx;
float my;
float mz;
float pitch;
float roll;
float yaw;

/* Timing related variables */
unsigned long dataReportInterval = 5000;
unsigned long previousReportTime = 0;
unsigned long resetTime;

/* Data logging related variables */
#define SD_CHIP_SELECT 23
#define SD_CARD_DETECT 45
#define RESET_INTERVAL 600000
#define TOTAL_DATA_FILENAME "DATA.CSV"

DataFile dataFile(MEGA, SD_CHIP_SELECT, SD_CARD_DETECT);
DataFile totalDataFile(MEGA, SD_CHIP_SELECT, SD_CARD_DETECT);
boolean dataLogging;
unsigned int filenameIndex;
boolean radioTransmitting;


void setup() 
{   
    /* Start the serial communication lines */
    Serial.begin(115200);
    Serial1.begin(9600);
    Serial2.begin(4800);

    /* Start the I2C bus */
    Wire.begin();

    /* Start the sensors the need initialization */
    pressureSensor.begin();
    pressureSensor.setModeBarometer();
    pressureSensor.setOversampleRate(7);
    pressureSensor.enableEventFlags();

    imu.settings.device.commInterface = IMU_MODE_I2C;
    imu.settings.device.mAddress = LSM9DS1_M;
    imu.settings.device.agAddress = LSM9DS1_AG;
    imu.begin();

    /* Clear out the array storing the ascent rate history */
    clearAscentHistory();

    /* Set the mode of each pin being used by the flight computer */
    pinMode(TRANSMISSION_PIN, INPUT);
    pinMode(TEMPERATURE_PIN_INTERIOR, INPUT);
    pinMode(TEMPERATURE_PIN_EXTERIOR, INPUT);
    pinMode(HUMIDITY_PIN, INPUT);
    pinMode(VOLTAGE_PIN, INPUT);


    /* Set up the current data file */
    dataFile.begin();
    setupFileEntries(dataFile);
    dataFile.writeFileHeader();
    filenameIndex = dataFile.getFilenameIndex();

    /* Set up the other data file (for all measurements) */
    totalDataFile.begin(TOTAL_DATA_FILENAME);
    setupFileEntries(totalDataFile);
    if (!totalDataFile.exists())
    {
        totalDataFile.writeFileHeader();  
    } else {
        totalDataFile.open();
        totalDataFile.writeNewLine();
        totalDataFile.close();
    }
}


void loop()
{
    /* Figure out if the radio is transmitting */
    radioTransmitting = (digitalRead(TRANSMISSION_PIN) == HIGH);
    
    /* Get the current battery voltage */
    batteryVoltage = getBatteryVoltage(VOLTAGE_PIN);
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
    
    /* Read the data from each of the temperature sensors */
    interiorTemperature1 = getTemperature(ds_interior);
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
    interiorTemperature2 = pressureSensor.readTemp();
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
    exteriorTemperature = getTemperature(ds_exterior);
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);

    /* Read the data from the pressure sensor */
    pressure = pressureSensor.readPressure();
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);

    /* Read the data from the humidity sensor */
    humidity = humiditySensor.getTrueRH(interiorTemperature1);
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);

    /** 
     * Read values from the GPS, including the most current time,
     * current lattitude and longitude, and altitude
     */
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
    radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);

    gpsValidity = (gps.time.age() < GPS_WARNING_AGE);
    gpsSentences = gps.sentencesWithFix();
    failedSentences = gps.failedChecksum();
    numSatellites = gps.satellites.value();
    gpsTime = (unsigned long) gps.time.value();
    gpsDate = (unsigned long) gps.date.value();

    getLatitudeString(gps);
    getLongitudeString(gps);
    rawAltitude = gps.altitude.value();
    altitude = rawAltitude / 100.0;

    rawLatitudeDegrees = gps.location.rawLat().deg;
    rawLatitudeBillionths = gps.location.rawLat().billionths;
    rawLatitudeSign = gps.location.rawLat().negative ? 0xFF : 1;
    
    rawLongitudeDegrees = gps.location.rawLng().deg;
    rawLongitudeBillionths = gps.location.rawLng().billionths;
    rawLongitudeSign = gps.location.rawLng().negative ? 0xFF : 1;

    getAscentRate();
    payloadSpeed = gps.speed.mps();

    /* Update the IMU sensor readings */
    if ( imu.gyroAvailable() ) {
        radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
        imu.readGyro();
    }
    if ( imu.accelAvailable() ){
        radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
        imu.readAccel();
    }
    if ( imu.magAvailable() ){
        radioTransmitting |= (digitalRead(TRANSMISSION_PIN) == HIGH);
        imu.readMag();
    }

    /* Get the readings from each of the sensors */
    ax = imu.calcAccel(imu.ax);
    ay = imu.calcAccel(imu.ay);
    az = imu.calcAccel(imu.az);
    gx = imu.calcGyro(imu.gx) + RATE_X;
    gy = imu.calcGyro(imu.gy) + RATE_Y;
    gz = imu.calcGyro(imu.gz) + RATE_Z;
    mx = imu.calcMag(imu.mx);
    my = imu.calcMag(imu.my);
    mz = imu.calcMag(imu.mz);

    /* Get the current attitude of the payload  */
    getAttitude(ax, ay, az, mx, my, mz, pitch, roll, yaw);

    /* Get the time since the last reset event */
    resetTime = millis();

    /* Log the data in the SD card, but only if we have enough time before reset */
    if (RESET_INTERVAL - millis() > 2000)
    {
        dataFile.open();
        dataLogging = dataFile.checkStatus();
        logData(dataFile);
        dataFile.close();

        totalDataFile.open();
        logData(totalDataFile);
        totalDataFile.close();  
    }

    /* Determine whether or not we need to send out another update
     *  to the serial debugging line and the communication microcontroller.
     */
    if (millis() - previousReportTime >= dataReportInterval)
    {
        numBytes = buildPacket(data);
        Serial1.write(data, numBytes);
        
        displayData();
        previousReportTime = millis();
    }
}
