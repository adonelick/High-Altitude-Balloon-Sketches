/**
 * Written by Andrew Donelick
 * 24 June 2017
 */


 void getAttitude(float ax, float ay, float az, float mx, float my, float mz, float& pitch, float& roll, float& yaw)
 {
    roll = atan2(ay, az);
    pitch = atan2(-ax, sqrt(ay * ay + az * az));

    if (my == 0) {
        yaw = (mx < 0) ? PI : 0;   
    } else {
        yaw = atan2(mx, my);
    }
    
    yaw -= DECLINATION * PI / 180;
    
    if (yaw > PI) {
        yaw -= (2 * PI);
    } else if (yaw < -PI) {
        yaw += (2 * PI);
    } else if (yaw < 0) {
        yaw += 2 * PI;
    }
    
    // Convert everything from radians to degrees:
    yaw *= 180.0 / PI;
    pitch *= 180.0 / PI;
    roll  *= 180.0 / PI;
 }

