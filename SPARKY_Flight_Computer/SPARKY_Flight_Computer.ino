// Written by Andrew Donelick
// 14 June 2014

#include <Wire.h>
#include <OneWire.h>
#include <String.h>
#include <SD.h>
#include <PacketRadio.h>
#include <BalloonCommands.h>

// Pins and settings for the analog sensors
#define HUMIDITY A0
#define INTERNAL_TEMP A1
#define HEATER_TEMP A2
#define NUM_READINGS 10

// Temperature calibration values (in 0.01 degrees C)
#define ANALOG_INTERIOR_OFFSET 200
#define HEATER_OFFSET 400

// Pins for the digital sensors
// Note that the BMP085 pressure sensor uses the 
// Wire interface (it's connected to SDA and SCL)
#define EXTERNAL_TEMP 4
#define BMP085_ADDRESS 0x77

// Relay pins
#define RELAY_1 49
#define RELAY_2 47
#define RELAY_3 45
#define RELAY_4 43

#define HEATER RELAY_4

// Radio modem pins
#define DSR 2
#define RTS 3

// The LED pin
#define LED 13

#define TURBULENCE 10

// Make the external temperature (OneWire) object
// Used in the Sensors.ino file
OneWire externalTemp(EXTERNAL_TEMP);

// Setup the radio
PacketRadio radio(Serial1, DSR, RTS);
char packet[2 * MAX_BUFFER_LENGTH];
unsigned int packetLength;
unsigned int recievedData[MAX_BUFFER_LENGTH];
unsigned int recievedDataLength;
boolean packetIntact;


// The communication protocol for my packet radio system:
//    _packet source_ (GROUND, BALLOON)
//    _packet type_ (COMMAND, REPORT, COMMAND_RESPONSE)
//    _command_ (some value, or NULL)
//    _value 1_
//    _value 2_
//    ...
//    _value n_

// This buffer holds the data to be sent by radio
unsigned int data[MAX_BUFFER_LENGTH];

// Create the objects necessary for writing to the SD card
File dataFile; // Stores the recoreded data from sensors
File logFile; // Stores computer operations in case of a crash
// Fileanme to which the data will be logged
char filename[] = "FLIGHT00.csv";
// Filename to which computer operations will be logged
char computerLogName[] = "COMPUTER.txt";

// Time between radio packet transmissions
unsigned long delayTime = 20000;

// Variables to hold all of the data measurements
unsigned int interiorTemp1;
unsigned int interiorTemp2;
unsigned int heaterTemp;
unsigned int exteriorTemp;
unsigned int pressure;
unsigned int humidity;
unsigned int h;

String attitude;
unsigned int pitch;
unsigned int roll;
unsigned int yaw;
boolean turbulence = false;

unsigned int flightDuration;

boolean heaterOn = false;
boolean dataLogging = true;
boolean manualHeaterControl = false;

// Stores the relay states
unsigned int relayStates;
boolean relay1 = false;
boolean relay2 = false;
boolean relay3 = false;
boolean relay4 = false;
boolean relay5 = false;
boolean relay6 = false;
boolean relay7 = false;
boolean relay8 = false;

// This value stores the last time data was sent over the radio
unsigned long lastTransmissionTime = 0;

void setup()
{
  
  // Delete any previous computer log files
  if (SD.exists(computerLogName)) {
    SD.remove(computerLogName);
  }
  
  // This sketch uses three serial ports:
  // Serial is used for debugging and output to computer
  // Serial1 is used for communicating with the radio
  // Serial2 is used for communicating with the Arduino interfaced with the GPS
  // Serial3 is used for communicating with the IMU
  
  // The radio modem (Serial1) requires a baud rate of 1200
  // The IMU (Serial3) requires a baud ratio of 57600
  
  logEntry("Initializing serial ports");
  Serial.begin(115200);
  Serial1.begin(1200);
  Serial2.begin(4800);
  Serial3.begin(57600);
  
  // Initialize the sensors
  Wire.begin();
  bmp085Calibration();
  
  // Initialize the relay pins, and make sure
  // all relays are currently OFF.
  
  // Relay_4 is the relay controlling the heater. All others
  // are currently disconnected.
  
  // The relays are weird - to turn them on, you need
  // to pull the pin to LOW, not HIGH.
  logEntry("Initializing relays");
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  
  // To start, all relays are off
  relayStates = 0;
  
  // Start the SD card
  // This pin needs to be set to output for writing to the
  // SD card, even if we do not use it
  pinMode(10, OUTPUT);
  pinMode(53, OUTPUT);
  SD.begin(53);
  
  logEntry("Finding new flight data logging file");
  // Allow for up to 100 flight files
  int i = 1;
  while (SD.exists(filename) && (i < 99))
  {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    ++i;
  }
  
  logEntry("Initializing data logging file with header");
  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile)
  {
    dataFile.print("Time,");
    dataFile.print("Interior Temperature 1,");
    dataFile.print("Time,");
    dataFile.print("Interior Temperature 2,");
    dataFile.print("Time,");
    dataFile.print("Heater Temperature,");
    dataFile.print("Time,");
    dataFile.print("External Temperature,");
    dataFile.print("Time,");
    dataFile.print("Humidity,");
    dataFile.print("Time,");
    dataFile.print("Pressure,");
    dataFile.print("Time,");
    dataFile.print("Pitch,");
    dataFile.print("Time,");
    dataFile.print("Roll,");
    dataFile.print("Time,");
    dataFile.print("Yaw,");
    dataFile.print("Time,");
    dataFile.print("Heater On?,");
    dataFile.print("Time,");
    dataFile.print("Relay States,");
    dataFile.print("Time,");
    dataFile.print("Altitude");
    dataFile.println();
    dataFile.close();
  }
}

void loop()
{ 
  dataFile = SD.open(filename, FILE_WRITE);
  
  if (dataFile)
  {
    logEntry("Opening data logging file succeeded");
    dataLogging = true;
    
    // Units of 0.01 degrees C (all temperatures)
    logEntry("Reading BMP085 temperature (interiorTemp1)");
    interiorTemp1 = 10 * bmp085GetTemperature(bmp085ReadUT());
    logEntry("Writing BMP085 temperature (interiorTemp1) to file");
    writeDataToFile(dataFile, interiorTemp1);
    
    logEntry("Reading analog temperature (interiorTemp2)");
    interiorTemp2 = 100 * getAnalogTemperature(INTERNAL_TEMP, NUM_READINGS) + ANALOG_INTERIOR_OFFSET;
    logEntry("Writing analog temperature (interiorTemp2) to file");
    writeDataToFile(dataFile, interiorTemp2);
    
    logEntry("Reading analog temperature (heaterTemp)");
    heaterTemp = 100 * getAnalogTemperature(HEATER_TEMP, NUM_READINGS) + HEATER_OFFSET;
    logEntry("Writing analog temperature (heaterTemp) to file");
    writeDataToFile(dataFile, heaterTemp);
    
    logEntry("Reading digital temperature (externalTemp)");
    exteriorTemp = 100 * getDigitalTemperature(externalTemp);
    logEntry("Writing digital temperature (exteriorTemp) to file");
    writeDataToFile(dataFile, exteriorTemp);
    
    // Units of 0.01 %
    logEntry("Reading analog humidity");
    humidity = 100 * getAnalogHumidity(HUMIDITY, NUM_READINGS);
    logEntry("Writing analog humidity to file");
    writeDataToFile(dataFile, humidity);
    
    // Units of 0.0001 psi
    logEntry("Reading BMP085 pressure");
    pressure = bmp085GetPressure(bmp085ReadUP())*0.145037738;
    logEntry("Writing BMP085 pressure to file");
    writeDataToFile(dataFile, pressure);
    
    // The attitude values are the true values
    /*
    logEntry("Reading attitude string");
    attitude = getAttitudeString();
    logEntry("Extracting pitch from attitude string");
    pitch = getPitch(attitude);
    logEntry("Writing pitch to file");
    writeDataToFile(dataFile, pitch);
    
    logEntry("Extracting roll from attitude string");
    roll = getRoll(attitude);
    logEntry("Writing roll to file");
    writeDataToFile(dataFile, roll);
    
    logEntry("Extracting yaw from attitude string");
    yaw = getYaw(attitude);
    logEntry("Writing yaw to file");
    writeDataToFile(dataFile, yaw);
    */
    
    logEntry("Ignoring attitude sensor readings");
    pitch = 0;
    logEntry("Writing pitch to file");
    writeDataToFile(dataFile, pitch);
    
    roll = 0;
    logEntry("Writing roll to file");
    writeDataToFile(dataFile, roll);
    
    yaw = 0;
    logEntry("Writing yaw to file");
    writeDataToFile(dataFile, yaw);

    logEntry("Writing heater status to file");
    dataFile.print(millis());
    dataFile.print(',');
    
    if (heaterOn) {
      dataFile.print("TRUE,");
    } else {
      dataFile.print("FALSE,");
    }
    
    logEntry("Writing relay states to file");
    dataFile.print(millis());
    dataFile.print(',');
    dataFile.print(relayStates);
    //dataFile.print(',');
    
    //dataFile.print(millis());
    //dataFile.print(',');
    //logEntry("Getting altitude from other Arduino");
    //h = getAltitude(Serial2);
    //logEntry("Writing altitude to file");
    //dataFile.print(h);
    
    
    dataFile.println();
    dataFile.close();
    
  } else {
    dataLogging = false;
    interiorTemp1 = 10 * bmp085GetTemperature(bmp085ReadUT());
    interiorTemp2 = 100 * getAnalogTemperature(INTERNAL_TEMP, NUM_READINGS) + ANALOG_INTERIOR_OFFSET;
    heaterTemp = 100 * getAnalogTemperature(HEATER_TEMP, NUM_READINGS) + HEATER_OFFSET;
    exteriorTemp = 100 * getDigitalTemperature(externalTemp);
    humidity = 100 * getAnalogHumidity(HUMIDITY, NUM_READINGS);
    pressure = bmp085GetPressure(bmp085ReadUP())*0.145037738;
    //pressure = 0;
    /*
    attitude = getAttitudeString();
    pitch = getPitch(attitude);
    roll = getRoll(attitude);
    yaw = getYaw(attitude);
    */
    pitch = 0;
    roll = 0;
    yaw = 0;
    //h = getAltitude(Serial2);
  }
  
  logEntry("Checking whether module is experiencing turbulence");
  turbulence = turbulence || (abs((int)roll) > TURBULENCE) || (abs((int)pitch) > TURBULENCE);
  
  logEntry("Determining  if the heater should be on or off");
  if (!manualHeaterControl)
  {
    // Do we need to turn on the heater?
    if ((bmp085GetTemperature(bmp085ReadUT()) < 0.0) && (!heaterOn))
    {
      heaterOn = true;
    } else if ((bmp085GetTemperature(bmp085ReadUT()) > 2.0) && heaterOn) {
      heaterOn = false;
    }
  }
  
  logEntry("Switching the heat on or off");
  if (heaterOn)
  {
    relay4 = true;
    digitalWrite(HEATER, LOW);
  } else {
    relay4 = false;
    digitalWrite(HEATER, HIGH);
  }
  
  // Store all the relay states in a single variable
  logEntry("Storing all relay states in a single unsigned int");
  relayStates = (unsigned int) relay1;
  relayStates = relayStates | (((unsigned int) relay2) << 1);
  relayStates = relayStates | (((unsigned int) relay3) << 2);
  relayStates = relayStates | (((unsigned int) relay4) << 3);
  relayStates = relayStates | (((unsigned int) relay5) << 4);
  relayStates = relayStates | (((unsigned int) relay6) << 5);
  relayStates = relayStates | (((unsigned int) relay7) << 6);
  relayStates = relayStates | (((unsigned int) relay8) << 7);
  
  logEntry("Building data report packet");
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
  data[12] = (unsigned int) heaterOn; // unsigned
  data[13] = (unsigned int) dataLogging; // unsigned
  data[14] = (unsigned int) manualHeaterControl; //unsigned
  data[15] = (unsigned int) turbulence; // unsigned
  data[16] = relayStates; // unsigned
  //data[17] = h;
  
  logEntry("Checking if new packet transmission must be sent");
  // Should we send a new packet?
  if ((millis() - lastTransmissionTime) >= delayTime)
  {
    logEntry("Sending data report radio packet");
    radio.sendData(data, 18);
    logEntry("Resetting last transmission time and turbulence");
    lastTransmissionTime = millis();
    turbulence = false;
    
    // This delay is included to allow any scrambled sensor 
    // readings caused by the high power radio transmission 
    // to settle down.
    delay(1000);
  }
  
  logEntry("Checking if there is an incoming radio transmission"); 
  if (radio.available()) {
    // Checks whether there is incoming data waiting at the radio modem
    logEntry("Recieving packet data from radio");
    packetIntact = radio.recieveData(packet, packetLength);
    
    if (packetIntact) {
      logEntry("Processing radio packet");
      packetIntact = radio.processData(packet, recievedData, recievedDataLength);
      
      if ((recievedData[0] == GROUND) && (recievedData[1] == COMMAND) && packetIntact)
      {
        logEntry("Echoing command from the ground");
        echoCommand(recievedData[2], recievedData[3]);
        logEntry("Deciding what to do with the recieved commmand");
        switch (recievedData[2]) {
          case CHANGE_TRANSMIT_RATE:
            // Changes the rate at which data is sent to the ground
            logEntry("Changing the radio transmission rate");
            delayTime = (unsigned long) recievedData[3];
            break;
          case MANUAL_HEATER_CONTROL:
            // Enables (or disables) manual control of heater
            logEntry("Changing manual heater control");
            manualHeaterControl = (recievedData[3]) ? 1 : 0;
            break;
          case TURN_HEATER_ON:
            logEntry("Turing heater on");
            heaterOn = true;
            break;
          case TURN_HEATER_OFF:
            logEntry("Turing heater off");
            heaterOn = false;
            break;
          case SWITCH_RELAYS:
            logEntry("Switching relay states");
            switchRelays(recievedData[3]);
            break;
          case CHECK_RADIO_CONNECTION:
            logEntry("Checking radio connection");
            // The connection is already echoed
            // If any other action are required to do the 
            // pre-flight radio check, put them hear
            break;
          default:
            // In the default, the command is not recognized.
            break;
        }
      }
    }
  }
 
  logEntry(" "); 
}

void echoCommand(unsigned int command, unsigned int value)
{
  data[0] = BALLOON; // Source
  data[1] = COMMAND_RESPONSE; // Packet type
  data[2] = command; // Command
  data[3] = value; // Value
  radio.sendData(data, 4);
  delay(1000);
}

void switchRelays(unsigned int relayStates)
{
  // Extract the relay commands
  relay1 = (relayStates & 0x0001) ? true : false;
  relay2 = (relayStates & 0x0002) ? true : false;
  relay3 = (relayStates & 0x0004) ? true : false;
  relay4 = (relayStates & 0x0008) ? true : false;
  relay5 = (relayStates & 0x0010) ? true : false;
  relay6 = (relayStates & 0x0020) ? true : false;
  relay7 = (relayStates & 0x0040) ? true : false;
  relay8 = (relayStates & 0x0080) ? true : false;
  
  // Right now, only relays 1-4 are wired, so
  // only they should be switched. Ignore
  // the other relay commands for now.
            
  if (relay1) {
    digitalWrite(RELAY_1, LOW);
  } else {
    digitalWrite(RELAY_1, HIGH);
  }
  
  if (relay2) {
    digitalWrite(RELAY_2, LOW);
  } else {
    digitalWrite(RELAY_2, HIGH);
  }
  
  if (relay3) {
    digitalWrite(RELAY_3, LOW);
  } else {
    digitalWrite(RELAY_3, HIGH);
  }
  
  if (relay4) {
    digitalWrite(RELAY_4, LOW);
  } else {
    digitalWrite(RELAY_4, HIGH);
  }
  
  // In the future, add other relay code here...
}

void logEntry(char entry[])
{
  logFile = SD.open(computerLogName, FILE_WRITE);
  logFile.println(entry);
  logFile.close();
}
