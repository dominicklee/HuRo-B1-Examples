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

int ledPin = 2;  //led pin
int servoPin = 26;  //servo pin
int fsrPin = 35;  //force sensor pin

Servo myservo;  // create servo object to control a servo
int currentPos = 0; // start at angle 0

void moveServo(int angle) {
    if (angle < 0 || angle > 180) {
      Serial.println("Invalid range!");
    }
    else {  //range is within 0-180
      Serial.print("Moving now to ");
      Serial.println(angle);
      
      myservo.write(angle);  //move servo to position
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
      const char* comm = root["command"]; // 

      if (strcmp(action, "data") == 0) {  //if data is received
        
        if (strcmp(comm, "ledON") == 0) {
          digitalWrite(ledPin, HIGH);
        }
        
        if (strcmp(comm, "ledOFF") == 0) {
          digitalWrite(ledPin, LOW);
        }

        if (strcmp(comm, "servo0") == 0) {
          moveServo(0);
        }

        if (strcmp(comm, "servo180") == 0) {
          moveServo(180);
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
  pinMode(ledPin, OUTPUT);
  
  myservo.setPeriodHertz(50);   // standard 50hz servo
  myservo.attach(servoPin, 800, 2300);
  
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
