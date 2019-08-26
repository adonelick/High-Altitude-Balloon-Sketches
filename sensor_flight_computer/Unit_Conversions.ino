/**
 * Written by Andrew Donelick
 * adonelick@hmc.edu
 * 
 * This file contains some useful unit conversions for use in the 
 * balloon payload.
 */


/**
 * Convert temperature from Celsius to Fahrenheit
 * 
 * :param temp: Temperature in deg C
 * 
 * :return: Temperauture in deg F
 */
double deg_C_to_F(double temp)
{
    return 1.8 * temp + 32.0;
}


/**
 * Convert temperature from Fahrenheit to Celsius
 * 
 * :param temp: Temperature in deg F
 * 
 * :return: Temperauture in deg C
 */
double deg_F_to_C(double temp)
{
    return  (5.0/9.0)*(temp - 32.0);
}


double cm_to_ft(double cm)
{
    return 0.0328084 * cm;
}
