#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Initate objects
WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial
#define sensorID "r123456"  // unique ID for your device

long lastSent = 0;  // number of ms since last sent
int counter = 0;  // for testing

int ledPin1 = 2;  //led pin
int servoPin1 = 26;  //servo pin
int servoPin2 = 25;  //servo pin
int servoPin3 = 33;  //servo pin
int servoPin4 = 32;  //servo pin
int fsrPin = 35;  //force sensor pin

Servo myservo1;  // create servo object to control a servo
Servo myservo2;  // create servo object to control a servo
Servo myservo3;  // create servo object to control a servo
Servo myservo4;  // create servo object to control a servo

void moveServo(int servo, int angle) {
    if (angle < 0 || angle > 180) {
      Serial.println("Invalid range!");
    }
    else {  //range is within 0-180
      Serial.print("Moving now to ");
      Serial.println(angle);

      if (servo == 1) {
        myservo1.write(angle);  //move servo to position
      } else if (servo == 2) {
        myservo2.write(angle);  //move servo to position
      } else if (servo == 3) {
        myservo3.write(angle);  //move servo to position
      } else if (servo == 4) {
        myservo4.write(angle);  //move servo to position
      }
      
      delay(1000);
    }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.println("[Websocket Disconnected]");
      break;

    case WStype_CONNECTED:
      {
        USE_SERIAL.println("[Websocket Connected]");
  
        // Introduce our device
        const size_t capacity = JSON_OBJECT_SIZE(3);
        DynamicJsonBuffer jsonBuffer(capacity);
        
        JsonObject& root = jsonBuffer.createObject();
        root["action"] = "newRobot";
        root["robotID"] = sensorID;
        root["secret"] = "password";
        String packet;
        root.printTo(packet);   // Fills it with  {"action":"newRobot","robotID":"r123456","secret":"password"}
        webSocket.sendTXT(packet);   //Sends it to introduce to the server 
      }
      break;

    case WStype_TEXT:
    {
      USE_SERIAL.printf("[WSS text]: %s\n", payload);

      String incoming = (char *)payload;  // {"action":"data","command":"ledON"}

      const size_t capacity = JSON_OBJECT_SIZE(2) + 90;
      DynamicJsonBuffer jsonBuffer(capacity);
      
      JsonObject& root = jsonBuffer.parseObject(incoming);
      
      const char* action = root["action"]; // "data"
      const char* comm = root["command"]; //  "{\"control\":\"led\",\"state\":\"on\"}"

      // Parse sub-level JSON
      DynamicJsonBuffer commandBuffer;
      JsonObject& command = commandBuffer.parseObject(comm);
      
      const char* control = command["control"]; // "led" or "servo"
      
      // Check if the "state" field is present
      if (command.containsKey("state")) {
        const char* state = command["state"]; // "on" or "off"
        // You can add code here to control the LED
        if (strcmp(control, "led1") == 0) {
          if (strcmp(state, "on") == 0) {
            // Turn on LED code
            digitalWrite(ledPin1, HIGH);
          } else {
            // Turn off LED code
            digitalWrite(ledPin1, LOW);
          }
        }
      }
      
      // Check if the "angle" field is present
      if (command.containsKey("angle")) {
        int angle = command["angle"]; // e.g. 90
        if (strcmp(control, "servo1") == 0) {
          // Set servo to the specified angle
          moveServo(1, angle);
        }
        if (strcmp(control, "servo2") == 0) {
          // Set servo to the specified angle
          moveServo(2, angle);
        }
        if (strcmp(control, "servo3") == 0) {
          // Set servo to the specified angle
          moveServo(3, angle);
        }
        if (strcmp(control, "servo4") == 0) {
          // Set servo to the specified angle
          moveServo(4, angle);
        }
      }
      
    }
      break;
    case WStype_BIN:
      break;
  }
}

void sendStatus(String msg) {
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["action"] = "pubRobot";
  root["robotID"] = sensorID;
  root["sensorVal"] = msg;
  
  String packet;
  root.printTo(packet);   // Fills it with  {"action":"newRobot","robotID":"r123456","secret":"password"}
  webSocket.sendTXT(packet);  //send it to all the subscribers of this device
}

void setup() {
  pinMode(ledPin1, OUTPUT);
  
  myservo1.setPeriodHertz(50);   // standard 50hz servo
  myservo1.attach(servoPin1, 800, 2300);

  myservo2.setPeriodHertz(50);   // standard 50hz servo
  myservo2.attach(servoPin2, 800, 2300);

  myservo3.setPeriodHertz(50);   // standard 50hz servo
  myservo3.attach(servoPin3, 800, 2300);

  myservo4.setPeriodHertz(50);   // standard 50hz servo
  myservo4.attach(servoPin4, 800, 2300);
  
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  USE_SERIAL.println("Connecting to WiFi...");
  delay(3000);

  WiFiMulti.addAP("YourWirelessName", "password"); //connect to the building WiFi

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100); //keeps waiting until we get a connection
  }

  webSocket.beginSSL("gyropalm.com", 3200);
  webSocket.onEvent(webSocketEvent);  //attached the callback
}

void loop() {
  webSocket.loop(); //keep running to stay alive

  if (millis() - lastSent > 500) {
    int sensor = analogRead(fsrPin);
    int normalVal = constrain(sensor, 0, 3740);
    normalVal = map(normalVal, 0, 3740, 0, 100);
    
    String test = "sensor: " + String(sensor);   // says hello world 1
    sendStatus(test);
    counter ++;
    
    lastSent = millis();  //sync up the last time we sent something
  }
  
}
