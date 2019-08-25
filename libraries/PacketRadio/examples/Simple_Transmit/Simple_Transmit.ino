// Radio Example 1 (Simple transmit)
// Written by Andrew Donelick
// <adonelick@hmc.edu>

// Include the radio libraries
#include <PacketRadio.h>
#include <BalloonCommands.h>

// Communication pins to key the radio's mic
// and listen for incoming transmissions
#define RTS 2
#define DSR 3

// Initialize the radio object
PacketRadio radio(Serial, DSR, RTS);
unsigned int packet[50];

void setup()
{
  // Radio communication is 1200 baud
  Serial.begin(1200);
  
  // Construct the packet to be sent
  packet[0] = GROUND;
  packet[1] = COMMAND;
  packet[2] = CUTDOWN;
  
  // Send the packet
  radio.sendData(packet, 3);
}

void loop()
{
  // Nothing to do here...
}
