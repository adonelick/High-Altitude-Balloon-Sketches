// Written by Andrew Donelick
// 16 May 2016
// adonelick@hmc.edu

#include <OneWire.h>
#include <Wire.h>
#include <SD.h>
#include <Sensors.h>
#include <RazorAHRS.h>
#include <DataFile.h>
#include <RelayModule.h>
#include <PacketRadio.h>
#include <BalloonCommands.h>
#include <SoftwareSerial.h>
#include "TinyGPS++.h"

// Define the names for the relays
#define RELAY_1 0
#define RELAY_2 1
#define RELAY_3 2
#define HEATER 3

// Radio modem pins
#define DSR 2
#define RTS 3

// Attitude control pins
#define ATTITUDE_PIN 24
#define ATTITUDE_SERIAL_RX 10
#define ATTITUDE_SERIAL_TX 11
#define ATTITUDE_RESET_PIN 41

// Actuation thresholds for attitude control
#define PITCH_THRESHOLD 0
#define ROLL_THRESHOLD 0
#define YAW_THRESHOLD 0

// Define the threshold for turbulence (units of 0.01 degrees)
#define TURBULENCE 1000

#define LATITUDE_LENGTH 50
#define LONGITUDE_LENGTH 50

// Define the pins for controlling the relays
int relayPins[] = {49, 47, 45, 43}; 

// Create objects which will interact with the hardware
Sensors sensors;
PacketRadio radio(Serial1, DSR, RTS, 20000);
RazorAHRS razor(Serial3);
DataFile dataFile(MEGA);
RelayModule relays(relayPins, 4);
TinyGPSPlus gps;
SoftwareSerial attitudeSerial(ATTITUDE_SERIAL_RX, ATTITUDE_SERIAL_TX);

// Create some more variables for the radio communications
unsigned int data[MAX_BUFFER_LENGTH];
char packet[2 * MAX_BUFFER_LENGTH];
unsigned int packetLength;
unsigned int recievedData[MAX_BUFFER_LENGTH];
unsigned int recievedDataLength;
boolean packetIntact;


// Variables to hold all of the data measurements
unsigned int interiorTemp1;
unsigned int interiorTemp2;
unsigned int heaterTemp;
unsigned int exteriorTemp;
unsigned int pressure;
unsigned int humidity;
unsigned long h;
unsigned int pitch;
unsigned int roll;
unsigned int yaw;
long longPitch;
long longRoll;
long longYaw;
unsigned int heaterStatus;
unsigned int relayStates;

// GPS related variables
unsigned long gpsTime;
char latitude[LATITUDE_LENGTH];
char longitude[LONGITUDE_LENGTH];
unsigned int rawLatitudeDegrees;
unsigned int rawLongitudeDegrees;
unsigned long rawLatitudeBillionths;
unsigned long rawLongitudeBillionths;
unsigned int rawLatitudeSign;
unsigned int rawLongitudeSign;

// Control variables
boolean heaterOn = false;
boolean dataLogging = true;
boolean manualHeaterControl = false;
boolean turbulence = false;
boolean attitudeControl = false;
long desiredYaw = 0;
unsigned int pGain = 50;
unsigned int iGain = 0;
unsigned int dGain = 20;

void setup()
{   
    // This sketch uses five serial ports:
    // Serial is used for debugging and output to computer
    // Serial1 is used for communicating with the radio
    // Serial2 is used for communicating with the GPS
    // Serial3 is used for communicating with the IMU
    // attitudeSerial is used for communicated with attitude control system

    // The radio modem (Serial1) requires a baud rate of 1200
    // The GPS (Serial2) requires a baud rate of 4800
    // The IMU (Serial3) requires a baud ratio of 57600
    // The attitude controller (attitudeSerial) requires a baud rate of 9600

    Serial.begin(115200);
    Serial2.begin(4800);
    attitudeSerial.begin(9600);

    // Initialize the objects
    razor.begin();
    sensors.begin();
    relays.begin();
    dataFile.begin();
    radio.begin();

    // Prepare for attitude control
    pinMode(ATTITUDE_PIN, OUTPUT);
    pinMode(ATTITUDE_RESET_PIN, OUTPUT);
    digitalWrite(ATTITUDE_PIN, LOW);
    digitalWrite(ATTITUDE_RESET_PIN, HIGH);

    // Set the measurements to be logged in the dataFile
    dataFile.addEntry("Interior Temperature 1");
    dataFile.addEntry("Interior Temperature 2");
    dataFile.addEntry("Heater Temperature");
    dataFile.addEntry("Exterior Temperature");
    dataFile.addEntry("Pressure");
    dataFile.addEntry("Humidity");
    dataFile.addEntry("Altitude");
    dataFile.addEntry("Pitch");
    dataFile.addEntry("Roll");
    dataFile.addEntry("Yaw");
    dataFile.addEntry("Heater Status");
    dataFile.addEntry("Relay States");
    dataFile.addEntry("GPS Time");
    dataFile.addEntry("Latitude");
    dataFile.addEntry("Longitude");
    dataFile.writeFileHeader();

    // Load the saved settings from the SD card
    loadSettings();
}


void loop()
{
    // Collect measurements from the instruments and write them
    // to the data file. Note: entries muse be written in the same
    // order as they were listed in the setup!
    dataFile.open();
    dataLogging = dataFile.checkStatus();

    interiorTemp1 = 10 * sensors.bmp085GetTemperature();
    dataFile.writeEntry(interiorTemp1);

    interiorTemp2 = 100 * sensors.getAnalogTemperature(INTERNAL_TEMP) + ANALOG_INTERIOR_OFFSET;
    dataFile.writeEntry(interiorTemp2);

    heaterTemp = 100 * sensors.getAnalogTemperature(HEATER_TEMP) + HEATER_OFFSET;
    dataFile.writeEntry(heaterTemp);

    exteriorTemp = 100 * sensors.getDigitalTemperature(EXTERNAL_TEMP);
    dataFile.writeEntry(exteriorTemp);

    pressure = sensors.bmp085GetPressure()*0.145037738;
    dataFile.writeEntry(pressure);

    humidity = 100 * sensors.getAnalogHumidity(ANALOG_HUMIDITY);
    dataFile.writeEntry(humidity);
    
    // Read values from the GPS, including the most current time,
    // current lattitude and longitude, and altitude
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
        
    h = (unsigned long) gps.altitude.value();
    dataFile.writeEntry(h);

    // Read the attitude angles from the Razor AHRS. All angles are 
    // converted to units of 0.01 degrees for saving and transmission
    if (razor.available()) {
        razor.decodeMessage();
    }

    pitch = 100 * razor.getPitch();
    dataFile.writeEntry(pitch);

    roll = 100 * razor.getRoll();
    dataFile.writeEntry(roll);

    yaw = 100 * razor.getYaw();
    dataFile.writeEntry(yaw);

    heaterStatus = (unsigned int) relays.getRelayState(HEATER);
    dataFile.writeEntry(heaterStatus);

    relayStates = relays.getRelayStates();
    dataFile.writeEntry(relayStates);

    gpsTime = (unsigned long) gps.time.value();
    dataFile.writeEntry(gpsTime);
    
    getLatitudeString(gps);
    dataFile.writeEntry(latitude);
    
    getLongitudeString(gps);
    dataFile.writeEntry(longitude);

    rawLatitudeDegrees = gps.location.rawLat().deg;
    rawLatitudeBillionths = gps.location.rawLat().billionths;
    rawLatitudeSign = gps.location.rawLat().negative ? 0xFFFF : 1;
    
    rawLongitudeDegrees = gps.location.rawLng().deg;
    rawLongitudeBillionths = gps.location.rawLng().billionths;
    rawLongitudeSign = gps.location.rawLng().negative ? 1 : 0xFFFF;

    dataFile.close();

    // Determine if we are experiencing turbulence
    turbulence = (abs((int)roll) > TURBULENCE) || (abs((int)pitch) > TURBULENCE);

    // Check if the internal temperature of the module has dropped
    // sufficiently to warrant turning the heater on. If it is getting a little
    // warm, turn the heater off.
    if (!manualHeaterControl) {
        if ((sensors.bmp085GetTemperature() < 0.0) && (!heaterOn)) {
            heaterOn = true;
        } else if ((sensors.bmp085GetTemperature() > 2.0) && heaterOn) {
            heaterOn = false;
        }
    }

    // Command the heater to turn on or off
    if (heaterOn) {
        relays.switchRelayOn(HEATER);
    } else {
        relays.switchRelayOff(HEATER);
    }
    
    // Since we may have just switched the heater on or off, update
    // the relayStates measurement
    relayStates = relays.getRelayStates();

    // Save the current settings to a file which can be read in the 
    // event that the flight computer is reset while running
    saveSettings();
    
    // Now, we have enough information to construct a data packet to send 
    // back to the ground. Build the packet:
    buildStatusPacket();

    // Check if enough time has passed since the last transmission of a packget.
    // If so, send another status report to the ground.
    sendStatusPacket();
    
    // Check if there is an incoming radio transmission
    if (radio.available())
    {
        // Checks if the incoming packet is complete. If it is, the 
        // packet and its length will be stored in the 'packet'
        // and 'packetLength' variables.
        packetIntact = radio.recieveData(packet, packetLength);
        
        if (packetIntact)
        {
            // Try to decode the packet. Any data that is recieved will be 
            // stored in the 'recievedData' variable.
            packetIntact = radio.processData(packet, recievedData, recievedDataLength);
            
            // Ensure we are recieving a transmission from the ground station,
            // not something else we intercepted accidentally
            if ((recievedData[0] == GROUND) && (recievedData[1] == COMMAND) && packetIntact)
            {
                // Echo the command to let the operators know it was received
                echoCommand(recievedData[2], recievedData[3]);
                
                // Act on the command which was sent
                processCommand(recievedData[2], recievedData[3]);
            }
        }
    }
}


