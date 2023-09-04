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

int ledPin = 2;
int servoPin = 26;

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

      String incoming = (char * )payload;
      
      const size_t capacity = JSON_OBJECT_SIZE(4) + 4780;
      DynamicJsonBuffer jsonBuffer(capacity);
      JsonObject& root = jsonBuffer.parseObject(incoming);

      const char* action = root["action"]; // "info"
      const char* comm = root["command"]; // "output"
      if (strcmp(action,"data") == 0) { //valid received packet
        if (strcmp(comm,"ledON") == 0) {
          digitalWrite(ledPin, HIGH);
        }
        if (strcmp(comm,"ledOFF") == 0) {
          digitalWrite(ledPin, LOW);
        }
        if (strcmp(comm,"servo0") == 0) {
          moveServo(0);
        }
        if (strcmp(comm,"servo80") == 0) {
          moveServo(80);
        }
        if (strcmp(comm,"servo90") == 0) {
          moveServo(90);
        }
        if (strcmp(comm,"servo100") == 0) {
          moveServo(100);
        }
        if (strcmp(comm,"servo180") == 0) {
          moveServo(180);
        }
        
      } else {
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
  myservo.attach(servoPin, 500, 2500);  // set pulse min to 500 uS, and max to 2500 uS
  
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

  if (millis() - lastSent > 1000) {
    String test = "hello world " + String(counter);   // says hello world 1
    sendStatus(test);
    counter ++;
    
    lastSent = millis();  //sync up the last time we sent something
  }
  
}
