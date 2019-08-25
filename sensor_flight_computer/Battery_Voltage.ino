/**
 * Written by Andrew Donelick
 * 24 June 2017
 */

/**
 * Returns the current battery voltage measured from a 
 * voltage divider connected to an analog pin.
 * 
 * :param voltagePin: Pin to read the data from
 * 
 * :return: current voltage of the power supply
 */
float getBatteryVoltage(int voltagePin)
{
    float rawVoltage = analogRead(voltagePin) * (5.0 / 1024);
    float current = rawVoltage / 10000.0;
    float batteryVoltage = current * 30000.0;

    return VOLTAGE_ADJUSTMENT * batteryVoltage;
}
