// Written by Andrew Donelick
// 16 May 2016

// This file contains functions which deal with transmitting status
// packets, receiving commands, echoing a received packet, etc.

void buildStatusPacket()
{
    data[0] = BALLOON; // Source
    data[1] = REPORT; // Packet type
    data[2] = 0; // Not a command, so no command
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
    data[18] = rawLatitudeDegrees;
    data[19] = (unsigned int) (rawLatitudeBillionths >> 16);
    data[20] = (unsigned int) (rawLatitudeBillionths & 0xFFFF);
    data[21] = rawLatitudeSign;
    data[22] = rawLongitudeDegrees;
    data[23] = (unsigned int) (rawLongitudeBillionths >> 16);
    data[24] = (unsigned int) (rawLongitudeBillionths & 0xFFFF);
    data[25] = rawLongitudeSign;
    data[26] = (unsigned int) attitudeControl;
}


void sendStatusPacket()
{
    if (radio.timeToSendPacket())
    {
        radio.sendData(data, 27);
        
        // This delay is included to allow any scrambled sensor 
        // readings caused by the high power radio transmission 
        // to settle down.
        delay(1000);
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


void processCommand(unsigned int command, unsigned int commandValue)
{
    switch (command) 
    {
        case CHANGE_TRANSMIT_RATE:
            // Changes the rate at which data is sent to the ground
            radio.setTransmissionDelay((unsigned long) commandValue);
            break;
        case MANUAL_HEATER_CONTROL:
            // Enables (or disables) manual control of heater
            manualHeaterControl = (commandValue) ? true : false;
            break;
        case TURN_HEATER_ON:
            heaterOn = true;
            break;
        case TURN_HEATER_OFF:
            heaterOn = false;
            break;
        case SWITCH_RELAYS:
            // Switch the relays, but only allow manual control over
            // relays 1 and 2, as 3 and 4 are in use for other things
            relayStates = (commandValue & 0x0003) | (relayStates & 0x000C);
            relays.switchRelays(relayStates);
            break;
        case CHECK_RADIO_CONNECTION:
            // The connection is already echoed
            // If any other action are required to do the 
            // pre-flight radio check, put them here
            break;
        case TURN_ATTITUDE_CONTROL_ON:
            // Turn the attitude controller on
            attitudeControl = true;
            digitalWrite(ATTITUDE_PIN, HIGH);
            relays.switchRelayOn(RELAY_3);
            break;
        case TURN_ATTITUDE_CONTROL_OFF:
            // Turn the attitude controller off
            attitudeControl = false;
            digitalWrite(ATTITUDE_PIN, LOW);
            relays.switchRelayOff(RELAY_3);
            break;
        case SET_YAW:
            // Set the desired yaw for the payload
            // using the attitude controller
            attitudeSerial.write('Y');
            sendLongBytes(commandValue);
            desiredYaw = (long) commandValue;
            break;
        case SET_P_GAIN:
            // Set the proportional gain for yaw control
            attitudeSerial.write('P');
            sendLongBytes(commandValue);
            pGain = (unsigned int) commandValue;
            break;
        case SET_I_GAIN:
            // Set the integral gain for yaw control
            attitudeSerial.write('I');
            sendLongBytes(commandValue);
            iGain = (unsigned int) commandValue;
            break;
        case SET_D_GAIN:
            // Set the derivative gain for yaw control
            attitudeSerial.write('D');
            sendLongBytes(commandValue);
            dGain = (unsigned int) commandValue;
            break;
        case RESET_ATTITUDE_CONTROLLER:
            // Send a signal to reset the attitude controlling Arduino
            digitalWrite(ATTITUDE_RESET_PIN, LOW);
            delay(10);
            digitalWrite(ATTITUDE_RESET_PIN, HIGH);
            break;
        default:
            // In the default, the command is not recognized.
            break;
    }
}


void sendLongBytes(long command)
{
    attitudeSerial.write((char) ((command >> 24)) & 0xFF);
    attitudeSerial.write((char) ((command >> 16)) & 0xFF);
    attitudeSerial.write((char) ((command >> 8)) & 0xFF);
    attitudeSerial.write((char) (command & 0xFF));
}


