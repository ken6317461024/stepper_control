#include <ArduinoJson.h>

#define dirPin 2
#define stepPin 3
int stepsPerAngle;
int angle;
int uSecondDelay;
StaticJsonDocument<200> stepper;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(115200);
  Serial.println("hi");
  inputString.reserve(200);
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  //stepsPerDegree = //stepsPerRevolution/360;
  angle=360;
  stepsPerAngle=8.88887*angle;
}

void loop() {
   if (stringComplete) {
    DeserializationError error = deserializeJson(stepper, inputString);
    if (error) {
    Serial.println(inputString);
    }
    else{
      int received_angle=stepper["angle"];
      stepsPerAngle=8.88887*received_angle;
      const char* received_direction=stepper["dir"];
      uSecondDelay=stepper["usDelay"];
     if(strcmp(received_direction,"clockwise")==0)
        digitalWrite(dirPin, LOW);
     else
        digitalWrite(dirPin, HIGH);
      
      for (int i = 0; i < stepsPerAngle; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(uSecondDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(uSecondDelay);
      }
      
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  

}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
