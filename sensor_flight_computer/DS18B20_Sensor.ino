/**
 * Written by Andrew Donelick
 * 24 June 2017
 */


/**
 * This function contained returns the temperature 
 * from one DS18S20 in degrees Fahrenheit. You need to specify
 * the OneWire instance associated with the temperature sensor.
 * 
 * :param ds: OneWire instance associated with the sensor
 * 
 * :return: current temperature in degrees celsius.
 */
float getTemperature(OneWire& ds)
{   
    byte data[12];
    byte addr[8];
    
    if ( !ds.search(addr)) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return -1000;
    }
    
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return -1000;
    }
    
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
        Serial.print("Device is not recognized");
        return -1000;
    }
    
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    /* Wait for temperature conversion to complete */
    delay(750);
    
    byte present = ds.reset();
    ds.select(addr); 
    ds.write(0xBE); // Read Scratchpad
    
    // We need 9 bytes
    for (int i = 0; i < 9; i++) { 
        data[i] = ds.read();
    }
    
    ds.reset_search();
    
    byte MSB = data[1];
    byte LSB = data[0];

    // Using two's compliment
    float tempRead = ((MSB << 8) | LSB); 
    float temperatureSum = tempRead / 16;
    
    return deg_C_to_F(temperatureSum);
}
