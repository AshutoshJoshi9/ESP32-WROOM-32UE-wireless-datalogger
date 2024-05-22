#include <WiFi.h>
#include <WebServer.h>

// Network credentials
const char* apSSID = "ESP32WROOM32UE_AP";
const char* apPassword = "abc12345678";

WebServer server(80);

const int analogPin = 34;

#define MAX_READINGS 1000
int readings[MAX_READINGS];
int readingIndex = 0;
bool isReading = false; // Flag to control reading

// HTML webpage
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Analog Reading</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    .button { display: inline-block; padding: 10px 20px; font-size: 20px; margin: 5px; cursor: pointer; }
    .button-start { background-color: #4CAF50; color: white; }
    .button-stop { background-color: #f44336; color: white; }
    .button-download { background-color: #008CBA; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 Analog Reading</h1>
  <p>Analog Value: <span id="analogValue">N/A</span></p>
  <p><button class="button button-start" onclick="startReading()">Start Reading</button></p>
  <p><button class="button button-stop" onclick="stopReading()">Stop Reading</button></p>
  <p><a href="/download"><button class="button button-download">Download CSV</button></a></p>
  <script>
    function startReading() {
      fetch('/start').then(response => response.text()).then(data => {
        console.log(data);
      });
    }
    
    function stopReading() {
      fetch('/stop').then(response => response.text()).then(data => {
        console.log(data);
      });
    }

    setInterval(function() {
      fetch('/analog').then(response => response.text()).then(data => {
        document.getElementById('analogValue').innerText = data;
      });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";


void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Function to handle refreshing values
void handleAnalog() {
  if (isReading) {
    int analogValue = analogRead(analogPin);
    if (readingIndex < MAX_READINGS) {
      readings[readingIndex++] = analogValue;
    }
    server.send(200, "text/plain", String(analogValue));
  } else {
    server.send(200, "text/plain", "Stopped");
  }
}

// Function to handle starting the reading
void handleStart() {
  isReading = true;
  server.send(200, "text/plain", "Reading Started");
}

// Function to handle stopping the reading
void handleStop() {
  isReading = false;
  server.send(200, "text/plain", "Reading Stopped");
}

// Function to handle downloading the CSV file
void handleDownload() {
  String csv = "Reading\n";
  for (int i = 0; i < readingIndex; i++) {
    csv += String(readings[i]) + "\n";
  }
  server.send(200, "text/csv", csv);
}

void setup() {
  
  Serial.begin(115200);
  delay(10);

  pinMode(analogPin, INPUT);

  Serial.println();
  Serial.println("Configuring access point...");

  // Initialize the WiFi as Access Point
  WiFi.softAP(apSSID, apPassword);

  // Print the IP address
  Serial.print("Access Point \"");
  Serial.print(apSSID);
  Serial.println("\" started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());


  server.on("/", handleRoot);
  server.on("/analog", handleAnalog);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/download", handleDownload);

  server.begin();
}

void loop() {
  server.handleClient();
}
