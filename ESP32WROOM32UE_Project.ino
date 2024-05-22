#include <WiFi.h>
#include <WebServer.h>

// Replace these with your network credentials
const char* apSSID = "ESP32_AP";
const char* apPassword = "12345678"; // Minimum 8 characters

// Create a WebServer object on port 80
WebServer server(80);

// ADC pin where the potentiometer is connected
const int analogPin = 34;

// Variables to store readings
#define MAX_READINGS 1000
int readings[MAX_READINGS];
int readingIndex = 0;

// HTML content for the web page
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Analog Reading</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    .button { display: inline-block; padding: 10px 20px; font-size: 20px; margin: 5px; cursor: pointer; }
    .button-refresh { background-color: #008CBA; color: white; }
    .button-download { background-color: #4CAF50; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 Analog Reading</h1>
  <p>Analog Value: <span id="analogValue">N/A</span></p>
  <p><a href="/refresh"><button class="button button-refresh">Refresh</button></a></p>
  <p><a href="/download"><button class="button button-download">Download CSV</button></a></p>
  <script>
    setInterval(function() {
      fetch('/analog').then(response => response.text()).then(data => {
        document.getElementById('analogValue').innerText = data;
      });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

// Function to handle the root path "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Function to handle refreshing the analog value
void handleAnalog() {
  int analogValue = analogRead(analogPin);
  if (readingIndex < MAX_READINGS) {
    readings[readingIndex++] = analogValue;
  }
  server.send(200, "text/plain", String(analogValue));
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
  // Start the serial communication to output the connection status
  Serial.begin(115200);
  delay(10);

  // Set the analog pin as an input (this is actually not necessary as analogRead implicitly sets the pin mode)
  pinMode(analogPin, INPUT);

  // Print starting message
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

  // Define the handling functions for specific paths
  server.on("/", handleRoot);
  server.on("/analog", handleAnalog);
  server.on("/download", handleDownload);

  // Start the server
  server.begin();
}

void loop() {
  // Handle client requests
  server.handleClient();
}
