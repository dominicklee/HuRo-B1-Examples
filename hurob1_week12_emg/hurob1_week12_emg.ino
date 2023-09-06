#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>
#include "hurob1_emg_model.h"

#define NUMBER_OF_INPUTS 500
#define NUMBER_OF_OUTPUTS 4
#define TENSOR_ARENA_SIZE 5 * 1024  // You might need to adjust this value
#define EMG_PIN 35

float inputs[NUMBER_OF_INPUTS];
float outputs[NUMBER_OF_OUTPUTS];
int rollingIndex = 0;

int mostLikelyGesture = -1;
float maxConfidence = 0.0;
int lastGesture = -1;
float lastConfidence = 0.0;

Eloquent::TinyML::TensorFlow::TensorFlow<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> eloquentModel;

long lastPrediction = 0;

void setup() {
  Serial.begin(115200);
  eloquentModel.begin(model_tflite);
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  int sensorV = analogRead(EMG_PIN);
  //Serial.println(sensorV);
  
  if (rollingIndex < NUMBER_OF_INPUTS) {
    inputs[rollingIndex] = map((float)sensorV, 0, 4096, 0, 3.30);
    rollingIndex++;
    delay(3);
  } else {
    for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
      inputs[i] = inputs[i + 1];
    }
    inputs[NUMBER_OF_INPUTS - 1] = map((float)sensorV, 0, 4096, 0, 3.30);
    delay(3);
  }
  
  if (millis() - lastPrediction > 300) {
    uint32_t start = micros();
    eloquentModel.predict(inputs, outputs);
    uint32_t duration = micros() - start;
    /*
    Serial.print("Took ");
    Serial.print(duration);
    Serial.println(" micros for inference");
    */
    mostLikelyGesture = -1;
    maxConfidence = 0.0;

    for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) { // iterate each result
      if (outputs[i] > maxConfidence) { // get the max confidence
        maxConfidence = outputs[i];
        mostLikelyGesture = i;
      }
    }
    
    if (maxConfidence > 0.29 && mostLikelyGesture == 0 && mostLikelyGesture != lastGesture) { // 30% confidence minimum for idle
       switch (lastGesture) { //user's last gesture
          case 1:
            // grab
            Serial.println("Grab gesture");
            Serial.print("Confidence: ");
            Serial.print(lastConfidence * 100);
            Serial.println("%");
            lastGesture = -1;
            break;
          case 2:
            // single tap
            Serial.println("Single Tap gesture");
            Serial.print("Confidence: ");
            Serial.print(lastConfidence * 100);
            Serial.println("%");
            break;
          case 3:
            // double tap
            Serial.println("Double Tap gesture");
            Serial.print("Confidence: ");
            Serial.print(lastConfidence * 100);
            Serial.println("%");
            break;
          default:
            // do nothing
            break;
        }
      
      Serial.println("Idle");
      lastGesture = mostLikelyGesture;
    } else {
      if (maxConfidence > 0.5 && mostLikelyGesture != lastGesture) {  // 50% min confidence for the rest
        lastGesture = mostLikelyGesture;  //hold that thought
        lastConfidence = maxConfidence;  //hold that thought
      }
    }
    /*
      Serial.print("Most likely gesture: ");
      Serial.println(mostLikelyGesture);
      Serial.print("Confidence: ");
      Serial.print(maxConfidence * 100);
      Serial.println("%");
    */
    lastPrediction = millis();
  }
}
