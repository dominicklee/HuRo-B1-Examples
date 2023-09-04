// Read data from Force Sensitive Resistor on D35

int fsrPin = 35;  //force sensor pin

void setup() {
  Serial.begin(115200);
}

void loop() {
  int fsr = analogRead(fsrPin);
  Serial.println(fsr);
  delay(100);
  
}
