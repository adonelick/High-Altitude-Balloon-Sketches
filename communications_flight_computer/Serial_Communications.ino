void receiveSensorData()
{
    dataPacket.clear();

    if (SENSOR_PORT.available() <= NUM_SENSOR_DATA_BYTES) {
        byte sensor_data[256];
        size_t numBytes = SENSOR_PORT.readBytes(sensor_data, NUM_SENSOR_DATA_BYTES);

        if (numBytes > 0)
        {
            for (size_t i = 0; i < numBytes; ++i)
            {
                dataPacket.addByte(sensor_data[i]);
            }        
            DEBUG_PORT.print("Received ");
            DEBUG_PORT.print(dataPacket.getNumBytes());
            DEBUG_PORT.println(" bytes from the sensor serial port");
            lastDataArrival = millis();
        }
    }
}

void receiveRadio(HardwareSerial& radio)
{
    /* This serial port connects with the radio */
    commandPacket.clear();

    boolean validKISS = true;
    unsigned int index = 0;
    byte newByte;
    while (radio.available() > 0)
    {
        newByte = radio.read();
        if ((index == 0) && (newByte != FEND)) {
            validKISS = false;
        } else if ((index == 1) && (newByte != 0x00)) {
            validKISS = false;
        }

        if ((index > 1) && validKISS) {
            commandPacket.addEscapedByte(newByte);
        }
        
        index++;
        delay(20);    
    }

    DEBUG_PORT.print("Received ");
    DEBUG_PORT.print(index);
    DEBUG_PORT.println(" bytes from Radio");
    
    commandPacket.descapeBytes();
    processCommand();

    /* Clear out any data that we may have received from Serial1 */
    while (radio.available() > 0)
    {
        radio.read();
        delay(2);
    }
}

void sendPacket(Packet& packet, byte packetType, HardwareSerial& radio)
{
    packet.escapeBytes();
    byte kissBytes[PACKET_SIZE];
    unsigned int numBytes = packet.getKISS(kissBytes, packetType);
    DEBUG_PORT.print("Sending ");
    DEBUG_PORT.print(numBytes);
    DEBUG_PORT.println(" bytes over the radio");

    digitalWrite(TRANSMISSION_PIN, HIGH);
    radio.write(kissBytes, numBytes);
    radio.flush();
    delay(2000);
    digitalWrite(TRANSMISSION_PIN, LOW);
}


void print_buffer(byte* data, size_t numBytes)
{
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        if ((i % 40 == 0) && (i != 0)) {
            DEBUG_PORT.println(" ");
        }
        
        if (data[i] < 0x10) {
            DEBUG_PORT.print("0");
        }
        DEBUG_PORT.print(data[i], HEX);
    }
    
    /* Print a separation line between data reports */
    DEBUG_PORT.println('\n');
}
