void receiveSerial1()
{
    /* This serial port connects with the sensor computer */
    dataPacket.clear();
    while (Serial1.available() > 0)
    {
        dataPacket.addByte(Serial1.read());
        delay(2);
    }
    dataPacket.escapeBytes();

    Serial.print("Received ");
    Serial.print(dataPacket.getNumBytes());
    Serial.println(" bytes from Serial 1");
    lastDataArrival = millis();
}

void receiveSerial2()
{
    /* This serial port connects with the radio */
    commandPacket.clear();

    boolean validKISS = true;
    unsigned int index = 0;
    byte newByte;
    while (Serial2.available() > 0)
    {
        newByte = Serial2.read();
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

    Serial.print("Received ");
    Serial.print(index);
    Serial.println(" bytes from Serial 2");
    
    commandPacket.descapeBytes();
    processCommand();

    /* Clear out any data that we may have received from Serial1 */
    while (Serial1.available() > 0)
    {
        Serial1.read();
        delay(2);
    }
}

void sendPacket(Packet& packet, byte packetType)
{
    packet.escapeBytes();
    byte kissBytes[PACKET_SIZE];
    unsigned int numBytes = packet.getKISS(kissBytes, packetType);
    Serial.print("Sending ");
    Serial.print(numBytes);
    Serial.println(" bytes over the radio");

    digitalWrite(TRANSMISSION_PIN, HIGH);
    Serial2.write(kissBytes, numBytes);
    Serial2.flush();
    delay(2000);
    digitalWrite(TRANSMISSION_PIN, LOW);
}

