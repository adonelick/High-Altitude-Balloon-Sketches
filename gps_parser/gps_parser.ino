/**
 * Written by Andrew Donelick
 * 25 August 2019
 * 
 * This file contains code to run the GPS sentence parsing computers. 
 */

#include <Arduino.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

#define TRANSMISSION_PERIOD 2000

static NMEAGPS  gps;
static gps_fix  fix;
static unsigned long last_transmission = 0;

void setup() 
{
    DEBUG_PORT.begin(4800);
    while (!DEBUG_PORT) {}
    gpsPort.begin( 4800 );
}

void loop() 
{
    if (gps.available(gpsPort)) 
    {
        fix = gps.read();
    }

    /**
     * Log the fix information if we have a location and time.
     * Keep this seperate from the update code, as we always want 
     * to be sending the most recent position data to the flight computers
     * so those computers don't have to wait on this system to parse the next
     * GPS sentence. 
     */ 

    byte binary_data[4];
    if (fix.valid.location && fix.valid.time && (millis() - last_transmission >= TRANSMISSION_PERIOD))
    {
        DEBUG_PORT.write(fix.dateTime.year);
        DEBUG_PORT.write(fix.dateTime.month);
        DEBUG_PORT.write(fix.dateTime.date);
        DEBUG_PORT.write(fix.dateTime.hours);
        DEBUG_PORT.write(fix.dateTime.minutes);
        DEBUG_PORT.write(fix.dateTime.seconds);

        int32_to_buffer(binary_data, fix.latitudeL());
        DEBUG_PORT.write(binary_data, 4); 
        
        int32_to_buffer(binary_data, fix.longitudeL());
        DEBUG_PORT.write(binary_data, 4); 

        int32_to_buffer(binary_data, fix.altitude_cm());
        DEBUG_PORT.write(binary_data, 4); 

        uint32_to_buffer(binary_data, fix.speed_metersph());
        DEBUG_PORT.write(binary_data, 4); 

        uint16_to_buffer(binary_data, fix.heading_cd());
        DEBUG_PORT.write(binary_data, 2); 

        DEBUG_PORT.write(fix.satellites);
        last_transmission = millis();
    }

}


void uint16_to_buffer(byte* binary_data, uint16_t x)
{    
    union 
    {
        uint16_t value;
        byte bytes[2];
    } converter;

    converter.value = x;
    memcpy(binary_data, converter.bytes, 2);
}


void int16_to_buffer(byte* binary_data, int16_t x)
{    
    union 
    {
        int16_t value;
        byte bytes[2];
    } converter;

    converter.value = x;
    memcpy(binary_data, converter.bytes, 2);
}


void uint32_to_buffer(byte* binary_data, uint32_t x)
{    
    union 
    {
        uint32_t value;
        byte bytes[4];
    } converter;

    converter.value = x;
    memcpy(binary_data, converter.bytes, 4);
}


void int32_to_buffer(byte* binary_data, int32_t x)
{    
    union 
    {
        int32_t value;
        byte bytes[4];
    } converter;

    converter.value = x;
    memcpy(binary_data, converter.bytes, 4);
}
