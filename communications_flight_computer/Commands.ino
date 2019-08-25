#define RADIO_TEST 1
#define SWITCH_RELAYS 3
#define RESET 5
#define CHANGE_TRANSMISSION_RATE 7
#define CUTDOWN 9

void processCommand()
{
    
    /* Test if the packet is a valid KISS packet */
    unsigned int numBytes = commandPacket.getNumBytes();

    if (numBytes < NUM_COMMAND_BYTES) {
        return;
    }

    /* Check if the packet is a command packet */
    if (commandPacket.getByte(0) == RADIO_COMMAND) 
    {
        Serial.print("Command Received: ");
        byte command = commandPacket.getByte(1);
        byte commandValueBytes[4];
        commandValueBytes[0] = commandPacket.getByte(2);
        commandValueBytes[1] = commandPacket.getByte(3);
        commandValueBytes[2] = commandPacket.getByte(4);
        commandValueBytes[3] = commandPacket.getByte(5);
        unsigned long commandValue = buildUnsignedLong(commandValueBytes);
        
        switch (command) 
        {
            case RADIO_TEST:
                Serial.println("RADIO_TEST");
                //buildAckPacket(RADIO_TEST, 0);
                //sendPacket(ackPacket, ACK);
                break;
                
            case CHANGE_TRANSMISSION_RATE:
                Serial.print("CHANGE_TRANSMISSION_RATE ");
                Serial.println(commandValue);
                transmissionRate = commandValue;
                //buildAckPacket(CHANGE_TRANSMISSION_RATE, transmissionRate);
                //sendPacket(ackPacket, ACK);
                break;

            case SWITCH_RELAYS:
                Serial.print("SWITCH_RELAYS ");
                Serial.println(commandValue);
                relayStates = (unsigned int) commandValue;
                relays.switchRelays(relayStates);
                //buildAckPacket(SWITCH_RELAYS, (unsigned long) relayStates);
                //sendPacket(ackPacket, ACK);
                break;
    
            case CUTDOWN:
                Serial.println("CUTDOWN");
                //buildAckPacket(CUTDOWN, 0);
                //sendPacket(ackPacket, ACK);
                break;
                
            case RESET:
                Serial.println("RESET");
                digitalWrite(RESET_PIN, LOW);
                delay(10);
                digitalWrite(RESET_PIN, HIGH);
                //buildAckPacket(RESET, 0);
                //sendPacket(ackPacket, ACK);
                break;
 
            default:
                Serial.println("UNRECOGNIZED");
                break;
        
        }      
    }
}

unsigned long buildUnsignedLong(byte* bytes)
{
    unsigned long byte1 = bytes[3];
    unsigned long byte2 = bytes[2];
    unsigned long byte3 = bytes[1];
    unsigned long result = bytes[0];

    result |= (byte3 << 8);
    result |= (byte2 << 16);
    result |= (byte1 << 24);

    return result;
}


