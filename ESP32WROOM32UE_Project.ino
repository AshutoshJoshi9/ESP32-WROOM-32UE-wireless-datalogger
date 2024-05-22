#include <WiFi.h>
#include <WebServer.h>

// Network credentials
const char* apSSID = "ESP32WROOM32UE_AP";
const char* apPassword = "abc12345678";

//WebServer object on port 80
WebServer server(80);

//ADC pin 
const int analogPin = 34;

//variables to store readings and timestamps
#define MAX_READINGS 1000
struct Reading {
  int value;
  String timestamp;
};
Reading readings[MAX_READINGS];
int readingIndex = 0;
bool isReading = false; //flag to control reading

//function to format the elapsed time in hh:mm:ss format
String formatTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  char timeString[10];
  snprintf(timeString, sizeof(timeString), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(timeString);
}

//HTML web page
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
  <h1>Mechanica Systems</h1>
  <h2>ESP32 WROOM 32UE Analog Reading</h2>
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

//function to handle the root path "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

//function to handle refreshing the values
void handleAnalog() {
  if (isReading) {
    int analogValue = analogRead(analogPin);
    if (readingIndex < MAX_READINGS) {
      readings[readingIndex].value = analogValue;
      readings[readingIndex].timestamp = formatTime(millis());
      readingIndex++;
    }
    server.send(200, "text/plain", String(analogValue));
  } else {
    server.send(200, "text/plain", "Stopped");
  }
}

//function for starting the reading
void handleStart() {
  isReading = true;
  server.send(200, "text/plain", "Reading Started");
}

//function for stopping the reading
void handleStop() {
  isReading = false;
  server.send(200, "text/plain", "Reading Stopped");
}

//function for downloading the CSV file
void handleDownload() {
  String csv = "Timestamp,Reading\n";
  for (int i = 0; i < readingIndex; i++) {
    csv += readings[i].timestamp + "," + String(readings[i].value) + "\n";
  }
  server.send(200, "text/csv", csv);
}

void setup() {

  Serial.begin(115200);
  delay(10);

  pinMode(analogPin, INPUT);

  Serial.println();
  Serial.println("Configuring access point...");

  //initializing the WiFi Access Point
  WiFi.softAP(apSSID, apPassword);

  //getting the IP address
  Serial.print("Access Point \"");
  Serial.print(apSSID);
  Serial.println("\" started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  //define the handling functions for specific paths
  server.on("/", handleRoot);
  server.on("/analog", handleAnalog);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/download", handleDownload);

  server.begin();
}

void loop() {
  //handling client requests
  server.handleClient();
}
