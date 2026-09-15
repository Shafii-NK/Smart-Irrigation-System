#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ThingSpeak.h>
#include <ESPAsyncWebServer.h>

// HTML code for the web interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Smart Irrigation System</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap');
    
    body {
      font-family: 'Roboto', sans-serif;
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      color: #f0f0f0;
      padding: 10px;
      min-height: 100vh;
    }

    .container {
      max-width: 1200px;
      margin: 0 auto;
      padding: 20px 10px;
    }

    header {
      text-align: center;
      margin-bottom: 40px;
      background: rgba(255, 255, 255, 0.08);
      padding: 20px;
      border-radius: 15px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
    }

    h1 {
      color: #4fc3f7;
      font-size: 2.5em;
      margin-bottom: 10px;
      text-shadow: 0 2px 10px rgba(79, 195, 247, 0.3);
    }

    .status-bar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.9em;
      color: #b0bec5;
      margin-top: 10px;
      flex-wrap: wrap;
      gap: 20px;
    }

    .refresh-indicator {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 5px 12px;
      background: rgba(76, 175, 80, 0.2);
      border-radius: 20px;
      border: 1px solid #4caf50;
      color: #4caf50;
    }

    .refresh-dot {
      width: 8px;
      height: 8px;
      background: #4caf50;
      border-radius: 50%;
      animation: pulse 1s infinite;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.3; }
    }

    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 25px;
      margin-bottom: 35px;
    }

    .widget {
      background: rgba(255, 255, 255, 0.08);
      border: 2px solid rgba(255, 255, 255, 0.1);
      border-radius: 15px;
      padding: 25px;
      text-align: center;
      backdrop-filter: blur(10px);
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      cursor: pointer;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
    }

    .widget:hover {
      transform: translateY(-8px);
      background: rgba(255, 255, 255, 0.12);
      border-color: rgba(79, 195, 247, 0.5);
      box-shadow: 0 12px 48px rgba(79, 195, 247, 0.15);
    }

    .widget h2 {
      font-size: 1.1em;
      color: #b0bec5;
      margin-bottom: 15px;
      text-transform: uppercase;
      letter-spacing: 1px;
      font-weight: 500;
    }

    .sensor-value {
      font-size: 2.8em;
      font-weight: 700;
      color: #4fc3f7;
      margin: 15px 0;
      text-shadow: 0 2px 10px rgba(79, 195, 247, 0.3);
    }

    .unit {
      font-size: 0.6em;
      color: #90caf9;
      margin-left: 5px;
    }

    .sensor-icon {
      font-size: 2.5em;
      margin-bottom: 10px;
      opacity: 0.7;
    }

    .status-container {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 25px;
      margin-bottom: 35px;
    }

    .status {
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      padding: 25px;
      border-radius: 15px;
      border: 2px solid;
      font-weight: 600;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
      min-height: 150px;
    }

    .status.on {
      background: linear-gradient(135deg, rgba(76, 175, 80, 0.15), rgba(102, 187, 106, 0.1));
      border-color: #66bb6a;
      color: #66bb6a;
    }

    .status.off {
      background: linear-gradient(135deg, rgba(244, 67, 54, 0.15), rgba(229, 57, 53, 0.1));
      border-color: #e53935;
      color: #e53935;
    }

    .status-label {
      font-size: 0.95em;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 12px;
      opacity: 0.8;
    }

    .status-state {
      font-size: 2.2em;
      font-weight: 700;
    }

    .status-icon {
      font-size: 3em;
      margin-top: 10px;
      opacity: 0.8;
    }

    .thresholds-section {
      background: rgba(255, 255, 255, 0.08);
      border: 2px solid rgba(255, 255, 255, 0.1);
      border-radius: 15px;
      padding: 25px;
      margin-bottom: 35px;
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
    }

    .thresholds-section h3 {
      color: #4fc3f7;
      margin-bottom: 20px;
      text-transform: uppercase;
      letter-spacing: 1px;
      font-weight: 500;
    }

    .threshold-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 20px;
    }

    .threshold-item {
      background: rgba(0, 0, 0, 0.3);
      padding: 15px;
      border-radius: 10px;
      border-left: 4px solid #4fc3f7;
    }

    .threshold-label {
      color: #b0bec5;
      font-size: 0.9em;
      margin-bottom: 8px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .threshold-value {
      color: #4fc3f7;
      font-size: 1.8em;
      font-weight: 600;
    }

    .footer {
      text-align: center;
      color: #90caf9;
      font-size: 0.85em;
      margin-top: 30px;
      padding-top: 20px;
      border-top: 1px solid rgba(255, 255, 255, 0.1);
    }

    /* Responsive Design */
    @media (max-width: 768px) {
      h1 {
        font-size: 1.8em;
      }

      .sensor-value {
        font-size: 2em;
      }

      .sensor-grid,
      .status-container {
        grid-template-columns: 1fr;
      }

      .status-bar {
        flex-direction: column;
        gap: 10px;
        align-items: flex-start;
      }

      .threshold-grid {
        grid-template-columns: 1fr;
      }
    }

    @media (max-width: 480px) {
      .container {
        padding: 10px 5px;
      }

      h1 {
        font-size: 1.5em;
      }

      header {
        padding: 15px;
      }

      .widget {
        padding: 15px;
      }

      .sensor-value {
        font-size: 1.8em;
      }

      .sensor-icon {
        font-size: 2em;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>🌱 Smart Irrigation System</h1>
      <div class="status-bar">
        <div class="refresh-indicator">
          <span class="refresh-dot"></span>
          <span id="refresh-status">Live: 2s</span>
        </div>
        <span id="last-update">Last update: now</span>
      </div>
    </header>

    <!-- Sensor Readings -->
    <div class="sensor-grid">
      <div class="widget">
        <div class="sensor-icon">💧</div>
        <h2>Soil Moisture</h2>
        <div class="sensor-value"><span id="moisture">0</span><span class="unit">%</span></div>
      </div>

      <div class="widget">
        <div class="sensor-icon">🌡️</div>
        <h2>Temperature</h2>
        <div class="sensor-value"><span id="temperature">0.0</span><span class="unit">°C</span></div>
      </div>

      <div class="widget">
        <div class="sensor-icon">💨</div>
        <h2>Humidity</h2>
        <div class="sensor-value"><span id="humidity">0.0</span><span class="unit">%</span></div>
      </div>
    </div>

    <!-- System Status -->
    <div class="status-container">
      <div class="status off" id="pump-status">
        <div class="sensor-icon">💧</div>
        <div class="status-label">Pump Status</div>
        <div class="status-state">OFF</div>
      </div>

      <div class="status off" id="fan-status">
        <div class="sensor-icon">❄️</div>
        <div class="status-label">Fan Status</div>
        <div class="status-state">OFF</div>
      </div>

      <div class="status off" id="heater-status">
        <div class="sensor-icon">🔥</div>
        <div class="status-label">Heater Status</div>
        <div class="status-state">OFF</div>
      </div>
    </div>

    <!-- Thresholds Display -->
    <div class="thresholds-section">
      <h3>⚙️ Active Thresholds</h3>
      <div class="threshold-grid">
        <div class="threshold-item">
          <div class="threshold-label">Moisture Threshold</div>
          <div class="threshold-value"><span id="moisture-threshold">40</span>%</div>
        </div>
        <div class="threshold-item">
          <div class="threshold-label">Temp Cooling</div>
          <div class="threshold-value"><span id="temp-cooling">26</span>°C</div>
        </div>
        <div class="threshold-item">
          <div class="threshold-label">Temp Heating</div>
          <div class="threshold-value"><span id="temp-heating">24</span>°C</div>
        </div>
        <div class="threshold-item">
          <div class="threshold-label">High Temp</div>
          <div class="threshold-value"><span id="temp-high">30</span>°C</div>
        </div>
      </div>
    </div>

    <footer class="footer">
      <p>Smart Irrigation System • Real-time monitoring and control</p>
    </footer>
  </div>

  <script>
    let updateCount = 0;
    const refreshInterval = 2000; // 2 seconds

    // Update time display
    function updateLastTime() {
      const now = new Date();
      const timeStr = now.toLocaleTimeString();
      document.getElementById('last-update').innerHTML = 'Last update: ' + timeStr;
    }

    // Fetch and update sensor data
    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          // Update sensor values
          document.getElementById('moisture').innerHTML = data.moisture;
          document.getElementById('temperature').innerHTML = data.temperature.toFixed(1);
          document.getElementById('humidity').innerHTML = data.humidity.toFixed(1);

          // Update pump status
          const pumpStatus = document.getElementById('pump-status');
          if (data.pumpStatus) {
            pumpStatus.classList.remove('off');
            pumpStatus.classList.add('on');
            pumpStatus.querySelector('.status-state').innerHTML = 'ON';
          } else {
            pumpStatus.classList.remove('on');
            pumpStatus.classList.add('off');
            pumpStatus.querySelector('.status-state').innerHTML = 'OFF';
          }

          // Update fan status
          const fanStatus = document.getElementById('fan-status');
          if (data.fanStatus) {
            fanStatus.classList.remove('off');
            fanStatus.classList.add('on');
            fanStatus.querySelector('.status-state').innerHTML = 'ON';
          } else {
            fanStatus.classList.remove('on');
            fanStatus.classList.add('off');
            fanStatus.querySelector('.status-state').innerHTML = 'OFF';
          }

          // Update heater status
          const heaterStatus = document.getElementById('heater-status');
          if (data.heaterStatus) {
            heaterStatus.classList.remove('off');
            heaterStatus.classList.add('on');
            heaterStatus.querySelector('.status-state').innerHTML = 'ON';
          } else {
            heaterStatus.classList.remove('on');
            heaterStatus.classList.add('off');
            heaterStatus.querySelector('.status-state').innerHTML = 'OFF';
          }

          // Update thresholds if provided
          if (data.moistureThreshold) {
            document.getElementById('moisture-threshold').innerHTML = data.moistureThreshold;
          }
          if (data.tempCooling) {
            document.getElementById('temp-cooling').innerHTML = data.tempCooling;
          }
          if (data.tempHeating) {
            document.getElementById('temp-heating').innerHTML = data.tempHeating;
          }
          if (data.tempHigh) {
            document.getElementById('temp-high').innerHTML = data.tempHigh;
          }

          updateLastTime();
        })
        .catch(error => {
          console.error('Error fetching data:', error);
        });
    }

    // Initial fetch and set up interval
    fetchData();
    setInterval(fetchData, refreshInterval);

    // Update refresh indicator
    setInterval(() => {
      updateCount++;
      if (updateCount % 30 === 0) {
        updateCount = 0;
      }
    }, 100);
  </script>
</body>
</html>
)rawliteral";

// Soil Moisture Sensor
const int soilMoisturePin = 35;

// DHT11 Sensor
#define DHTPIN 12 // Pin where the data line is connected
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Relay Pin
const int relayPin = 15;

//Fan Pin
const int fanPin1 = 2;
const int fanPin2 = 4;
const int fanEN = 5;

//Heater Pin
const int heater = 18;

// ============================================================
// SECURITY: Configure WiFi and ThingSpeak credentials below
// ============================================================
// Wi-Fi credentials
char ssid[] = "YOUR_SSID_HERE";           // CHANGE THIS
char pass[] = "YOUR_PASSWORD_HERE";       // CHANGE THIS

// ThingSpeak credentials
unsigned long myChannelNumber = 0;         // CHANGE THIS
const char * myWriteAPIKey = "YOUR_API_KEY_HERE"; // CHANGE THIS
// ============================================================

// Adjusted threshold for inverse mapping
const int moistureThreshold = 40; // Moisture percentage threshold for inverse mapping
const float tempHighThreshold = 30.0; // High temperature in Celsius
const float humidityLowThreshold = 40.0; // Low humidity percentage
const float tempCoolingThreshold = 26.0; // Fan cooling temperature
const float tempHeatingThreshold = 24.0; // Heater activation temperature

WiFiClient client;
AsyncWebServer server(80);

// Variables to store sensor values
float temperature = 0.0;
float humidity = 0.0;
int moisturePercent = 0;
bool pumpStatus = false; // Track the status of the pump
bool fanStatus = false; // Track the status of the fan
bool heaterStatus = false; // Track the status of the heater
unsigned long lastThingSpeakUpdate = 0; // Track the last ThingSpeak update time

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize the DHT sensor
  dht.begin();

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initial setup for relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure the pump is off initially

  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);
  pinMode(fanEN, OUTPUT);
  pinMode(heater, OUTPUT);

  digitalWrite(fanEN, HIGH);
  digitalWrite(fanPin1, HIGH);
  digitalWrite(fanPin2, HIGH);
  digitalWrite(heater, LOW);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Route to get sensor data
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temperature\":" + String(temperature) + ",";
    json += "\"humidity\":" + String(humidity) + ",";
    json += "\"moisture\":" + String(moisturePercent) + ",";
    json += "\"pumpStatus\":" + String(pumpStatus ? "true" : "false") + ",";
    json += "\"fanStatus\":" + String(fanStatus ? "true" : "false") + ",";
    json += "\"heaterStatus\":" + String(heaterStatus ? "true" : "false") + ",";
    json += "\"moistureThreshold\":" + String(moistureThreshold) + ",";
    json += "\"tempCooling\":" + String(tempCoolingThreshold) + ",";
    json += "\"tempHeating\":" + String(tempHeatingThreshold) + ",";
    json += "\"tempHigh\":" + String(tempHighThreshold);
    json += "}";
    request->send(200, "application/json", json);
  });

  // Start server
  server.begin();
}

void loop() {
  // Read the analog value from the soil moisture sensor
  int sensorValue = analogRead(soilMoisturePin);

  // Convert the analog value to an inverse percentage
  moisturePercent = map(sensorValue, 4095, 0, 0, 100);

  // Read humidity and temperature values from DHT11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Control the water pump based on thresholds
  if (moisturePercent > moistureThreshold) {
    digitalWrite(relayPin, HIGH); // Turn on the water pump
    pumpStatus = true;
  } else if (temperature > tempHighThreshold && humidity < humidityLowThreshold) {
    digitalWrite(relayPin, HIGH); // Turn on the water pump
    pumpStatus = true;
  } else {
    digitalWrite(relayPin, LOW); // Turn off the water pump
    pumpStatus = false;
  }

  // Temperature control - improved logic
  if (temperature > tempCoolingThreshold) {
    // Cooling mode: turn on fan
    digitalWrite(fanPin1, HIGH);
    digitalWrite(fanPin2, LOW);
    digitalWrite(heater, LOW);
    fanStatus = true;
    heaterStatus = false;
  }
  else if (temperature < tempHeatingThreshold) {
    // Heating mode: turn on heater and stop fan
    digitalWrite(fanPin1, LOW);
    digitalWrite(fanPin2, LOW);
    digitalWrite(heater, HIGH);
    fanStatus = false;
    heaterStatus = true;
  }
  else {
    // Neutral zone: turn off all systems
    digitalWrite(fanPin1, LOW);
    digitalWrite(fanPin2, LOW);
    digitalWrite(heater, LOW);
    fanStatus = false;
    heaterStatus = false;
  }

  // Print values to the Serial Monitor
  Serial.print("Soil Moisture (Inverted): ");
  Serial.print(moisturePercent);
  Serial.println("%");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("C");

  // Upload data to ThingSpeak every 15 seconds
  if (millis() - lastThingSpeakUpdate >= 15000) {
    ThingSpeak.setField(1, moisturePercent);
    ThingSpeak.setField(2, temperature);
    ThingSpeak.setField(3, humidity);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code: " + String(x));
    }

    lastThingSpeakUpdate = millis();
  }

  delay(2000); // Delay before the next loop iteration
}
