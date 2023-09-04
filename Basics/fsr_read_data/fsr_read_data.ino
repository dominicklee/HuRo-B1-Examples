// Read data from Force Sensitive Resistor on D35

int fsrPin = 35;  //force sensor pin

void setup() {
  pinMode(fsrPin, INPUT);
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  int fsr = analogRead(fsrPin);
  Serial.println(fsr);
  delay(3);
  
}
