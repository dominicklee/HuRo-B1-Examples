#include <ESP32Servo.h>

int servoPin = 26;
Servo myservo;  // create servo object to control a servo
int currentPos = 0; // start at angle 0

void setup() {
  Serial.begin(115200);
  
  myservo.setPeriodHertz(50);   // standard 50hz servo
  myservo.attach(servoPin, 500, 2300);  // set pulse min to 500 uS, and max to 2500 uS
}

void moveServo(int angle) {
    if (angle < 0 || angle > 180) { // make sure range is valid first
      Serial.println("Invalid range!");
    }
    else {  //range is within 0-180
      Serial.print("Moving now to ");
      Serial.println(angle);
      
      myservo.write(angle);  //move servo to position
      delay(1000);
    }
}

void loop() {
  if (Serial.available() > 0) {
    currentPos = Serial.parseInt();
    moveServo(currentPos);
  }
}
