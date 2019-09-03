#define COMM_RESET_PIN 2
#define SENSOR_RESET_PIN 3
#define GPS_RESET_PIN 4
#define RESET_INTERVAL 600000

unsigned long lastReset = 0;

void setup()
{
    pinMode(COMM_RESET_PIN, OUTPUT);
    pinMode(SENSOR_RESET_PIN, OUTPUT);
    pinMode(GPS_RESET_PIN, OUTPUT);
    digitalWrite(COMM_RESET_PIN, HIGH);
    digitalWrite(SENSOR_RESET_PIN, HIGH);
    digitalWrite(GPS_RESET_PIN, HIGH);    
}

void loop() 
{
    if ((millis() - lastReset) > RESET_INTERVAL)
    {
        resetComputer(COMM_RESET_PIN);
        resetComputer(SENSOR_RESET_PIN);
        resetComputer(GPS_RESET_PIN);
        lastReset = millis();
    }

    delay(100);
}

void resetComputer(int resetPin)
{
    digitalWrite(resetPin, LOW);
    delay(10);
    digitalWrite(resetPin, HIGH);
}
