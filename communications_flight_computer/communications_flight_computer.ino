#include <RelayModule.h>
#include <Packet.h>

#define TRANSMISSION_PIN 13
#define RESET_PIN 2
#define NUM_COMMAND_BYTES 29

#define RELAY_1 4
#define RELAY_2 5
#define RELAY_3 6
#define RELAY_4 7

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

/* Data reporting variables */
unsigned long transmissionRate = 30000;
unsigned long previousReportTime = 0;
unsigned long lastDataArrival = 0;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(9600);
    Serial2.begin(1200);

    pinMode(TRANSMISSION_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);

    digitalWrite(TRANSMISSION_PIN, LOW);
    digitalWrite(RESET_PIN, HIGH);
    relays.begin();
}

void loop()
{
    relayStates = relays.getRelayStates();

    if ((Serial1.available() > 0) && (Serial2.available() > 0)) {
        receiveSerial2();
    } else if (Serial2.available() > 0) {
        receiveSerial2();
    } else if (Serial1.available() > 0) {
        receiveSerial1();
    }

    /* Every so often, send out a data report packet */
    if (millis() - previousReportTime >= transmissionRate)
    {
        /* Fill the data packet with necessary information */
        updateDataPacket();

        /* Send the packet off, record the time of transmission */
        sendPacket(dataPacket, DATA);
        previousReportTime = millis(); 

        /* Clear out the data packet for the next report */
        dataPacket.clear();
    }
}

