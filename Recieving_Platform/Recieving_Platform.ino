// Written by Andrew Donelick
// 15 June 2014

#include <PacketRadio.h>
#include <String.h>

// Radio modem communication pins
#define RTS 2
#define DSR 3

// Initialize the radio modem
PacketRadio radio(Serial1, DSR, RTS);

// The communication protocol for my packet radio system:
//    _packet source_ (GROUND, BALLOON)
//    _packet type_ (COMMAND, REPORT, COMMAND_RESPONSE)
//    _command_ (some value, or NULL)
//    _value 1_
//    _value 2_
//    ...
//    _value n_

// Data communication buffers
char packet[100];
char serialData[100];

unsigned int data[100];
unsigned int command[100];
unsigned int packetLength;
unsigned int dataLength;

boolean packetIntact;
String dataString;

int index;

void setup()
{
  // Initialize the radio's serial port here - it
  // is not initialized in the PacketRadio class
  Serial.begin(115200);
  Serial1.begin(1200);
}


void loop()
{  
  // Checks whether there is incoming data waiting at the radio modem
  if (radio.available()) {
    packetIntact = radio.recieveData(packet, packetLength);
    
    if (packetIntact) {
      packetIntact = radio.processData(packet, data, dataLength);
    
      dataString = "";
      for (int i = 0; i < dataLength; ++i)
      {
        dataString += data[i];
        dataString += " ";
      }
      
      Serial.print(dataString);
    }
  }
  
  // We are now recieving a command from the computer
  if (Serial.available())
  {
     index = 0;
     while (Serial.available())
     {
       serialData[index] = Serial.read();
       ++index;
     }
          
     command[0] = GROUND; // source
     command[1] = COMMAND; // type
     command[2] = (unsigned int) (serialData[0] << 8) | (serialData[1] & 0x00FF); // command
     command[3] = (unsigned int) (serialData[2] << 8) | (serialData[3] & 0x00FF); // value
     
     radio.sendData(command, 4);
  }
  
  delay(10);
  
}
