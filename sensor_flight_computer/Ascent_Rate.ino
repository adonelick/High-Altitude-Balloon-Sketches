void clearAscentHistory()
{
    for (unsigned int i = 0; i < ASCENT_RATE_VALUES; ++i)
    {
        ascentRateHistory[i] = 0.0; 
    }
}


void getAscentRate()
{    
    altitudeTime = millis();
    float tempAscentRate = (altitude - previousAltitude) / ((altitudeTime - previousAltitudeTime) / 1000.0);
    ascentRateHistory[(ascentRateIndex++) % ASCENT_RATE_VALUES] = tempAscentRate;

    previousAltitude = altitude;
    previousAltitudeTime = altitudeTime;

    ascentRate = 0;
    for (unsigned int i = 0; i < ASCENT_RATE_VALUES; ++i)
    {
        ascentRate += ascentRateHistory[i]; 
    }
    ascentRate /= (1.0 * ASCENT_RATE_VALUES);
}

