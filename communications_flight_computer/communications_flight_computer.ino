#include <RelayModule.h>
#include <Packet.h>

#define CUTDOWN_ALTITUDE 90000

#define TRANSMISSION_PIN 41
#define RESET_PIN 40
#define NUM_COMMAND_BYTES 29
#define NUM_SENSOR_DATA_BYTES 108

// Define the serial ports for the various interfaces 
#define DEBUG_PORT Serial
#define RADIO_1_PORT Serial2
#define RADIO_2_PORT Serial3
#define GPS_PORT Serial
#define SENSOR_PORT Serial1
#define SERIAL_TIMEOUT 5000

// Settings for using the two radios
#define USE_RADIO_1 false
#define USE_RADIO_2 false

// Pin assignments for the relays
#define RELAY_1 25
#define RELAY_2 27
#define RELAY_3 29
#define RELAY_4 31
#define CUTDOWN_RELAY RELAY_1

/* Relay variables */
int relayPins[] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};
RelayModule relays(relayPins, 4);
unsigned int relayStates;

Packet dataPacket;
Packet commandPacket;
Packet ackPacket;   

unsigned int dataPacketBytes;
unsigned int commandPacketBytes;
unsigned int kissPacketBytes;
bool sensor_data_received;

/* Data reporting variables */
unsigned long transmissionRate = 20000;
unsigned long previousReportTime = 0;
unsigned long lastDataArrival = 0;

// Data variables for the GPS data
uint8_t year;       /* Current year [00 - 99] */
uint8_t month;      /* Current month [01 - 12] */
uint8_t date;       /* Current date of month [01 - 31] */
uint8_t hour;       /* Current hour [0 - 23] */
uint8_t minute;     /* Current minute [0 - 59] */
uint8_t second;     /* Current second [0 - 59] */
int32_t latitude;   /* Latitude of the payload [] */
int32_t longitude;  /* Longitude of the payload [] */
int32_t altitude;   /* Altitude of the payload [cm] */
uint32_t speed;     /* Speed of the payload [meter/hour] */
uint16_t heading;   /* Heading of the payload [hundredths of degrees] */
uint8_t satellites; /* Number of satellites for the GPS */


void setup()
{
    DEBUG_PORT.begin(4800);
    SENSOR_PORT.begin(115200);
    RADIO_1_PORT.begin(1200);
    RADIO_2_PORT.begin(1200);
    GPS_PORT.begin(4800);

    pinMode(TRANSMISSION_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);

    digitalWrite(TRANSMISSION_PIN, LOW);
    digitalWrite(RESET_PIN, HIGH);
    relays.begin();
}

void loop()
{
    relayStates = relays.getRelayStates();

    if ((RADIO_1_PORT.available() > 0) && USE_RADIO_1) {
        receiveRadio(RADIO_1_PORT);
    } else if ((RADIO_2_PORT.available() > 0) && USE_RADIO_2) {
        receiveRadio(RADIO_2_PORT);
    } else if (GPS_PORT.available() > 0) {
        get_GPS_data();
    } else if (SENSOR_PORT.available() > 0) {
        receiveSensorData();
    }
    
    /* Every so often, send out a data report packet */
    if (millis() - previousReportTime >= transmissionRate)
    {
        /* Fill the data packet with necessary information */
        updateDataPacket();

        /* Send the packet off, record the time of transmission */
        sendPacket(dataPacket, DATA, RADIO_1_PORT);
        previousReportTime = millis(); 

        /* Clear out the data packet for the next report */
        dataPacket.clear();
        sensor_data_received = false;
    }
}
