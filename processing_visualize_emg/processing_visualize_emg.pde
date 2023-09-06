import controlP5.*;
import processing.serial.*;

Serial myPort;
ControlP5 cp5;
Textlabel statusLabel;  // Declare status label
Textlabel voltsLabel;  // Declare status label
ArrayList<Integer> dataPoints = new ArrayList<Integer>();
String portName = "";
int maxVal = 100;
boolean isFrozen = false;
boolean isRecording = false;
String recordFilename = "";  // Filename for recording (don't touch)
PrintWriter writer;
int counter = 0;  // Counter for incoming data points (don't touch)
float compressionRate = 0.9; // Compression rate, can be changed dynamically

int numSamples = 200;  // Collect number of samples for averaging
int voltRefreshPeriod = 150;  // Time in milliseconds for calculating and displaying avg
int curSample = 0;  // (don't touch)
long lastDisplayed = 0; // (don't touch)
float [] voltReadings = new float[numSamples];  // Array to collect voltage readings



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
     .setSize(100, 40);
     
  cp5.addButton("Record")
     .setPosition(645, 20)
     .setSize(100, 40);

  // Add status label
  statusLabel = cp5.addLabel("Status: Idle")
                   .setPosition(15, height - 30)
                   .setFont(p);
                   
  // Add volts label
  voltsLabel = cp5.addLabel("Volts: N/A")
                   .setPosition(width - 85, height - 30)
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
        
        // Calculate and log the voltage
        float volts = map(sensorVal, 0, 4096, 0, 3.3);  // convert 12-bit value to volts
        logSample(volts);  // stores the reading into array
        
        if (isRecording) {
          handleRecording(volts);
        }
        
        // Show avg voltage (every X period)
        if (millis() - lastDisplayed > voltRefreshPeriod) {
          float avgVolts = getAvgVoltage();
          String voltsString = String.format(java.util.Locale.US,"%.2f", avgVolts);  // round to 2 decimal
          showVoltage(voltsString);
          
          lastDisplayed = millis();  // update our flag
        }
        
        // Add the latest scaled data point
        int scaledVal = (int)map(sensorVal, 0, maxVal, 5, 530);  // 5 and 530 is height with margins
        
        counter++;  // Increment counter for each new data point
        // Add data point to array only if the counter is a multiple of 1/compressionRate
        if (counter % (int)(1 / compressionRate) == 0) {
          dataPoints.add(scaledVal);  // add point to the graph
        }

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

void Record() {
  if (myPort == null || isFrozen) { 
    updateStatus("Failed to record. Check serial or unfreeze data.");
    return;
  }
  isRecording = !isRecording; // Toggle the isRecording flag
  
  if (isRecording) {  // Update button label based on the new state
    cp5.getController("Record").setLabel("Stop");
    updateStatus("Recording now...");
    
    // Format the date and time to create the filename
    String dateTimeString = new java.text.SimpleDateFormat("MMddYYYY hh-mm-ss a").format(new java.util.Date());
    recordFilename = "EMG " + dateTimeString + ".csv";
    
    // Initialize writer when you start recording
    writer = createWriter(recordFilename);
    writer.println("EMG");
  } else {
    cp5.getController("Record").setLabel("Record");
    updateStatus("Recording stopped");
    // Stop recording logic
    
    if (writer != null) {
      writer.flush(); // Writes the remaining data to the file
      writer.close(); // Finishes the file
      writer = null;
    }
  }
}

void handleRecording(float data) {
  if (writer != null) {
    writer.println(data);
  }
}

void updateStatus(String alert) {
  statusLabel.setText("Status: " + alert);
}

void showVoltage(String volts) {
  voltsLabel.setText("Volts: " + volts);
}

void logSample(float volts) {
  // If curSample hasn't reached numSamples, add the new voltage to the array and increment curSample.
  if (curSample < numSamples) {
    voltReadings[curSample] = volts;
    curSample++;
  } else {
    // Shift all elements to the left by 1, effectively removing the oldest
    for (int i = 1; i < numSamples; i++) {
      voltReadings[i - 1] = voltReadings[i];
    }
    // Add the new voltage reading at the end of the array
    voltReadings[numSamples - 1] = volts;
  }
}

float getAvgVoltage() {
  float sum = 0;  // variable to store the cumulative total
  for (int i = 0; i < curSample; i++) {  // iterate the array
    sum += voltReadings[i];  // add each value from array
  }
  return sum/curSample;  // divide sum by n items
}