// Written by Andrew Donelick
// 14 December 2015
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
#include "TinyGPS++.h"

// Define the names for the relays
#define RELAY_1 0
#define RELAY_2 1
#define RELAY_3 2
#define HEATER 3

// Radio modem pins
#define DSR 2
#define RTS 3

// Define the threshold for turbulence (units of 0.01 degrees)
#define TURBULENCE 500

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
unsigned int heaterStatus;
unsigned int relayStates;
unsigned long gpsTime;
char latitude[LATITUDE_LENGTH];
char longitude[LONGITUDE_LENGTH];

boolean heaterOn = false;
boolean dataLogging = true;
boolean manualHeaterControl = false;
boolean turbulence = false;

void setup()
{  
    // This sketch uses three serial ports:
    // Serial is used for debugging and output to computer
    // Serial1 is used for communicating with the radio
    // Serial2 is used for communicating with the Arduino interfaced with the GPS
    // Serial3 is used for communicating with the IMU

    // The radio modem (Serial1) requires a baud rate of 1200
    // The IMU (Serial3) requires a baud ratio of 57600

    Serial.begin(115200);
    Serial2.begin(4800);

    // Initialize the objects
    razor.begin();
    sensors.begin();
    relays.begin();
    dataFile.begin();
    radio.begin();

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
    while (Serial2.available() > 0)
    {
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

    dataFile.close();

    // Determine if we are experiencing turbulence
    turbulence = turbulence || (abs((int)roll) > TURBULENCE) || (abs((int)pitch) > TURBULENCE);

    // Check if the internal temperature of the module has dropped
    // sufficiently to warrant turning the heater on. If it is getting a little
    // warm, turn the heater off.
    if (!manualHeaterControl)
    {
        if ((sensors.bmp085GetTemperature() < 0.0) && (!heaterOn))
        {
            heaterOn = true;
        } 
        else if ((sensors.bmp085GetTemperature() > 2.0) && heaterOn) {
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
    
    // Now, we have enough information to construct a data packet to send 
    // back to the ground. Build the packet:
    
    data[0] = BALLOON; // Source
    data[1] = REPORT; // Packet type
    data[2] = NULL; // Not a command, so no command
    data[3] = interiorTemp1; // signed
    data[4] = interiorTemp2; // signed
    data[5] = heaterTemp; // signed
    data[6] = exteriorTemp; // signed
    data[7] = humidity; // unsigned
    data[8] = pressure; // unsigned
    data[9] = pitch; // signed
    data[10] = roll; // signed
    data[11] = yaw; // signed
    data[12] = (unsigned int) gps.altitude.meters();
    data[13] = (unsigned int) heaterOn; // unsigned
    data[14] = (unsigned int) dataLogging; // unsigned
    data[15] = (unsigned int) manualHeaterControl; //unsigned
    data[16] = (unsigned int) turbulence; // unsigned
    data[17] = relayStates; // unsigned

    // Check if enough time has passed since the last transmission of a packget.
    // If so, send another status report to the ground.
    
    if (radio.timeToSendPacket())
    {
        radio.sendData(data, 18);
        
        // This delay is included to allow any scrambled sensor 
        // readings caused by the high power radio transmission 
        // to settle down.
        delay(1000);
    }
    
    
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
                // Echo the command to let the operators know it was recieved
                echoCommand(recievedData[2], recievedData[3]);
                
                switch (recievedData[2]) 
                {
                    case CHANGE_TRANSMIT_RATE:
                        // Changes the rate at which data is sent to the ground
                        radio.setTransmissionDelay((unsigned long) recievedData[3]);
                        break;
                    case MANUAL_HEATER_CONTROL:
                        // Enables (or disables) manual control of heater
                        manualHeaterControl = (recievedData[3]) ? true : false;
                        break;
                    case TURN_HEATER_ON:
                        heaterOn = true;
                        break;
                    case TURN_HEATER_OFF:
                        heaterOn = false;
                        break;
                    case SWITCH_RELAYS:
                        relays.switchRelays(recievedData[3]);
                        break;
                    case CHECK_RADIO_CONNECTION:
                        // The connection is already echoed
                        // If any other action are required to do the 
                        // pre-flight radio check, put them here
                        break;
                    default:
                        // In the default, the command is not recognized.
                        break;
                }
            }
        }
    }
}


void echoCommand(unsigned int command, unsigned int commandValue)
{
    data[0] = BALLOON; // Source
    data[1] = COMMAND_RESPONSE; // Packet type
    data[2] = command; // Command
    data[3] = commandValue; // Value
    radio.sendData(data, 4);
    delay(1000);
}




