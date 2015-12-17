// Written by Andrew Donelick
// 14 June 2014

#include <String.h>

/*
 * Gets the string from the AHRS that contains all the
 * attitude information (pitch, roll, yaw)
 */

String getAttitudeString()
{
  String attitudeString;
  char inByte;
  boolean stringComplete = false;
  boolean buildingString = false;
  boolean doneReading = false;
  
  unsigned long timeout;
  
  char byteArray[60];
  
  int index = 0;
  while (!doneReading)
  {
    if (Serial3.available()) {
      byteArray[index] = Serial3.read();
      index++;
      
      if (index == 60) {
        doneReading = true;
      }
    }
  }
  
 int i = 0;
 timeout = millis();
 while ((!stringComplete) && (millis() - timeout < 1000))
  {
    if (byteArray[i] == '#') {
      if (buildingString) {
        attitudeString = "";
      }
      buildingString = true;
    }
    
    if (buildingString) {
      attitudeString += byteArray[i];
      
      if (byteArray[i] == '\n') {
        stringComplete = true;
      }  
    }
    
    i++;
  }
  
  if (!(intact(attitudeString)))
  {
    attitudeString = "#YPR=0.00,0.00,0.00\n";
  }
  
  return attitudeString;
}

boolean intact(String attitudeString)
{
  boolean test1 = (attitudeString.substring(0, 5) == "#YPR=");
  
  int commaCount = 0;
  for (int i = 0; i < attitudeString.length(); ++i)
  {
    if (attitudeString[i] == ',')
    {
      ++commaCount;
    }
  }
  
  boolean test2 = (commaCount == 2);
  
  boolean test3 = (attitudeString[attitudeString.length() - 1] == '\n');
  return test1 && test2 && test3;
}
  

// For each of the following functions, the attitude string is
// indexed first at index 5, to skip over the "#YPR=" stuff.

// The following function extract specific orientation information 
// from the attitude string

int getPitch(String attitudeString)
{
  int index = 5;
  int commaCount = 0;
  char currentChar;
  String pitchString;
  
  while (commaCount < 1) 
  {
    if (attitudeString[index] == ',') {
      commaCount++;
    }
    index++;
  }
  
  currentChar = attitudeString[index];
  while (currentChar != ',')
  {
    pitchString += currentChar;
    index++;
    currentChar = attitudeString[index];
  }
  
  return pitchString.toInt(); 
}

int getRoll(String attitudeString)
{
  int index = 5;
  int commaCount = 0;
  char currentChar;
  String rollString;
  
  while (commaCount < 2) 
  {
    if (attitudeString[index] == ',') {
      commaCount++;
    }
    index++;
  }
  
  currentChar = attitudeString[index];
  while (currentChar != '\n')
  {
    rollString += currentChar;
    index++;
    currentChar = attitudeString[index];
  }
  
  // The +2 degrees is meant to account for the slight
  // tilt induced by gluing the IMU to the circuit board
  return rollString.toInt() + 2; 
}

int getYaw(String attitudeString)
{
  int index = 5;
  int commaCount = 0;
  char currentChar;
  String yawString;
  
  currentChar = attitudeString[index];
  while (currentChar != ',')
  {
    yawString += currentChar;
    index++;
    currentChar = attitudeString[index];
  }
  
  return yawString.toInt(); 
}


