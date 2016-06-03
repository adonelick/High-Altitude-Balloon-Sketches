// Written by Andrew Donelick
// 15 December 2015

#define RESET_PIN 13

// Reset the computer every 5 minutes
unsigned long delayTime = 300000;
unsigned long lastResetTime = 0;

void setup()
{
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, HIGH);
}


void loop()
{
    if ((millis() - lastResetTime) > delayTime)
    {
        digitalWrite(RESET_PIN, LOW);
        delay(50);
        digitalWrite(RESET_PIN, HIGH);
        lastResetTime = millis();
    }

    delay(1000);    
}
