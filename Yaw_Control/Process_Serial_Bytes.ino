// Written by Andrew Donelick
// adonelick@hmc.edu

void processSerial()
{
    byte byteArray[10];
    
    if (mySerial.available() > 4) {
        int index = 0;
        while (mySerial.available()) {
            byteArray[index] = mySerial.read();
            ++index;
        }
    } else {
        return;
    }

    switch (byteArray[0])
    {
        case DESIRED_YAW:
            // Change the desired yaw targeted by the attitude controller
            desiredYaw = getLongFromBytes(byteArray);
            attitudeController.setDesiredState(0, 0, desiredYaw);
            break;
        case P_GAIN:
            // Change the proportional gain for yaw control
            proportionalGain = getLongFromBytes(byteArray);
            attitudeController.setGains(YAW, proportionalGain, integralGain, derivativeGain);
            break;
        case I_GAIN:
            // Change the integral gain for yaw control
            integralGain = getLongFromBytes(byteArray);
            attitudeController.setGains(YAW, proportionalGain, integralGain, derivativeGain);
            break;
        case D_GAIN:
            // Change the derivative gain for yaw control
            derivativeGain = getLongFromBytes(byteArray);
            attitudeController.setGains(YAW, proportionalGain, integralGain, derivativeGain);
            break;
        default:
            // The command is not recognized, so do nothing
            break;
    }
}


long getLongFromBytes(byte bytes[])
{
    long returnValue = 0;
    for (int i = 1; i < 5; ++i) {
        returnValue = returnValue << 8;
        returnValue |= (long) bytes[i];
    }

    return returnValue;
}

