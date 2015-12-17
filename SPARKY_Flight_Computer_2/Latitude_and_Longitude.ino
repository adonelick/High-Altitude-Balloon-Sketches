// Written by Andrew Donelick
// 15 December 2015


void getLatitudeString(TinyGPSPlus& gps)
{
    latitude[0] = gps.location.rawLat().negative ? '-' : ' ';
    
    // Set up some other arrays for storing the reversed versions
    // of the degree and decimal portions of the latitude
    char reversedDecimal[LATITUDE_LENGTH];
    char reversedDegree[LATITUDE_LENGTH];
    
    unsigned int degree = gps.location.rawLat().deg;
    unsigned long decimals = gps.location.rawLat().billionths;
    unsigned int degreeLength = 0;
    unsigned int decimalLength = 0;
    
    unsigned int index = 0;
    while (degree != 0)
    {
       reversedDegree[index] = (degree % 10) + '0';
       ++index;
       degree /= 10;
    }
    degreeLength = index;

    index = 0;
    while (decimals != 0)
    {
       reversedDecimal[index] = (decimals % 10) + '0';
       ++index;
       decimals /= 10;
    }
    decimalLength = index;
    
    // Construct the final latitude string
    index = 1;
    for (int i = 0; i < degreeLength; ++i)
    {
        latitude[index] = reversedDegree[degreeLength - i - 1];
        ++index;    
    }
    
    latitude[index] = '.';
    ++index;

    for (int i = 0; i < decimalLength; ++i)
    {
        latitude[index] = reversedDecimal[decimalLength - i - 1];
        ++index;    
    }    
}


void getLongitudeString(TinyGPSPlus& gps)
{
    longitude[0] = gps.location.rawLng().negative ? '-' : ' ';
    
    // Set up some other arrays for storing the reversed versions
    // of the degree and decimal portions of the latitude
    char reversedDecimal[LONGITUDE_LENGTH];
    char reversedDegree[LONGITUDE_LENGTH];
    
    unsigned int degree = gps.location.rawLng().deg;
    unsigned long decimals = gps.location.rawLng().billionths;
    unsigned int degreeLength = 0;
    unsigned int decimalLength = 0;
    
    unsigned int index = 0;
    while (degree != 0)
    {
       reversedDegree[index] = (degree % 10) + '0';
       ++index;
       degree /= 10;
    }
    degreeLength = index;

    index = 0;
    while (decimals != 0)
    {
       reversedDecimal[index] = (decimals % 10) + '0';
       ++index;
       decimals /= 10;
    }
    decimalLength = index;
    
    // Construct the final latitude string
    index = 1;
    for (int i = 0; i < degreeLength; ++i)
    {
        longitude[index] = reversedDegree[degreeLength - i - 1];
        ++index;    
    }
    
    longitude[index] = '.';
    ++index;

    for (int i = 0; i < decimalLength; ++i)
    {
        longitude[index] = reversedDecimal[decimalLength - i - 1];
        ++index;    
    }    
}
