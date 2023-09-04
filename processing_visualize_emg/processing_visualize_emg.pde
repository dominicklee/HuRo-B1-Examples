import controlP5.*;
import processing.serial.*;

Serial myPort;
ControlP5 cp5;
Textlabel statusLabel;  // Declare status label
ArrayList<Integer> dataPoints = new ArrayList<Integer>();
String portName = "";
int maxVal = 100;
boolean isFrozen = false;
int counter = 0;  // Counter for incoming data points (don't touch)
float compressionRate = 0.6; // Compression rate, can be changed dynamically


void setup() {
  size(800, 600);
  surface.setTitle("HuRo-B1 | EMG Visualizer");
  cp5 = new ControlP5(this);

  PFont p = createFont("Arial", 16); // New font size
  cp5.setFont(p);

  // Create dropdown list for COM ports
  ListBox l = cp5.addListBox("COM Port")
                 .setPosition(20, 20)
                 .setSize(200, 200)
                 .setItemHeight(40)
                 .setBarHeight(40)
                 .close();  //keep it closed by default

  // Set bigger font for ListBox items
  l.getCaptionLabel().setFont(p).toUpperCase(false);
  l.getValueLabel().setFont(p).toUpperCase(false);

  String[] portNames = Serial.list();
  for (int i = 0; i < portNames.length; i++) {
    l.addItem(portNames[i], i);
  }

  // Connect/Disconnect button
  cp5.addButton("Connect")
     .setPosition(235, 20)
     .setSize(140, 40);

  cp5.addButton("Disconnect")
     .setPosition(385, 20)
     .setSize(140, 40);
     
  cp5.addButton("Freeze")
     .setPosition(535, 20)
     .setSize(140, 40);

  // Add status label
  statusLabel = cp5.addLabel("Status: Idle")
                   .setPosition(15, height - 30)
                   .setFont(p);
}

void draw() {
  background(0);
  stroke(255);
  strokeWeight(1);

  // Drawing the grid
  for (int x = 0; x < width; x += 40) {
    stroke(60);
    line(x, 0, x, height);
  }
  
  for (int y = 0; y < height; y += 40) {
    stroke(60);
    line(0, y, width, y);
  }
  
  strokeWeight(2);  // Make the green line thicker
  stroke(0, 255, 0); // Green color for the waveform
/*
  // Draw previous data points
  for (int i = 1; i < dataPoints.size(); i++) {
    line(i-1, height - dataPoints.get(i-1), i, height - dataPoints.get(i));
  }
  */

  if (myPort != null && isFrozen != true) {  // If we are connected to Serial
    while (myPort.available() > 0) {
      String inString = myPort.readStringUntil('\n');
      if (inString != null) {
        inString = inString.trim();
        int sensorVal = int(inString);  // Parse string to int
        
        // Auto-scale the waveform by getting max value
        if (sensorVal > maxVal) {
          maxVal = sensorVal;  //update maxVal 
        }
        
        // Add the latest scaled data point
        int scaledVal = (int)map(sensorVal, 0, maxVal, 5, 530);  // 5 and 530 is height with margins
        
        counter++;  // Increment counter for each new data point
        // Add data point to array only if the counter is a multiple of 1/compressionRate
        if (counter % (int)(1 / compressionRate) == 0) {
          dataPoints.add(scaledVal);
        }
        //dataPoints.add(scaledVal);  // add point to the graph

        // Remove the oldest data point to keep a constant number of points
        float maxPointsInWindow = 1.0 / compressionRate;
        if (dataPoints.size() > width * maxPointsInWindow) {
          dataPoints.remove(0);
        }
      }
    }
  }
  
// Draw previous data points, accounting for compression
for (int i = 1; i < dataPoints.size(); i++) {
  float x1 = (i - 1) * compressionRate;
  float x2 = i * compressionRate;
  line(x1, height - dataPoints.get(i-1), x2, height - dataPoints.get(i));
}

}

void controlEvent(ControlEvent theEvent) {
  if (theEvent.getName().equals("COM Port")) {
    int selected = int(theEvent.getValue());
    portName = Serial.list()[selected];
    updateStatus(portName + " selected");
    ((ListBox) cp5.getController("COM Port")).close();  // Close the list box after selection
  }
}

void Connect() {
  if (portName != "") {
    println(portName);
    myPort = new Serial(this, portName, 115200);
    updateStatus("Connected");
  } else {
    updateStatus("No Port Selected");
  }
}

void Disconnect() {
  if (myPort != null) {
    myPort.stop();
  }
  updateStatus("Disconnected");
}

void Freeze() {
  if (isFrozen) {
    isFrozen = false;
    updateStatus("Freeze Disabled");
  } else {
    isFrozen = true;
    updateStatus("Freeze Enabled");
  }
}

void updateStatus(String alert) {
  statusLabel.setText("Status: " + alert);
}