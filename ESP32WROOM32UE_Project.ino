#include <WiFi.h>
#include <WebServer.h>

//Network credentials
const char* apSSID = "ESP32WROOM32UE_AP";
const char* apPassword = "abc12345678"; // Minimum 8 characters

// Create a WebServer object on port 80
WebServer server(80);

const int analogPin = 34;

// HTML web page
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
  </style>
</head>
<body>
  <h1>ESP32 Analog Reading</h1>
  <p>Analog Value: <span id="analogValue">N/A</span></p>
  <p><a href="/refresh"><button class="button button-refresh">Refresh</button></a></p>
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


void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Function to handle refreshing values
void handleAnalog() {
  float analogValue = analogRead(analogPin) / 1000.0;
  server.send(200, "text/plain", String(analogValue));
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

  // Define the handling functions for specific paths
  server.on("/", handleRoot);
  server.on("/analog", handleAnalog);

  server.begin();
}

void loop() {
  
  server.handleClient();
}
