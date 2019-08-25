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

static NMEAGPS  gps;
static gps_fix  fix;

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
     
    if (fix.valid.location && fix.valid.time)
    {
        DEBUG_PORT.print(fix.dateTime.year);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.dateTime.month);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.dateTime.date);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.dateTime.hours);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.dateTime.minutes);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.dateTime.seconds);
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.latitudeL());
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.longitudeL());
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.altitude_cm());
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.speed_metersph());
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.heading_cd());
        DEBUG_PORT.print(",");
        DEBUG_PORT.print(fix.satellites);
        DEBUG_PORT.print('\n');
    }
}
