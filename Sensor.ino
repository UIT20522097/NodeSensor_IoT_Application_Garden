#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

// Config WiFi
const char* ssid = "QiQi";
const char* password = "UyenLe2401";

//config cho Moisture Sensor 
#define moisturePin 36
float moisture;
int valueMois;

//config cho Temperature and Humidity sensor
#define DHTPIN 4                                    // Khai báo chân kết nối của DHT11
#define DHTTYPE DHT11                               // Khai báo loại cảm biến DHT11
DHT dht(DHTPIN, DHTTYPE);
float humidity;                                     // Khai báo biến lưu trữ độ ẩm
float temperature; 

// Config cho Light sensor
#define lightPin 39 // Chân kết nối của cảm biến ánh sáng

// Create a WiFiClient object to establish the connection
WiFiClient client;

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password); // Replace with your Wi-Fi credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  //Moisture sensor
  analogReadResolution(12);                         // set ADC resolution to 12 bits (0-4095) 
  pinMode(moisturePin, INPUT);
  
  //Tempurate and humidity sensor 
  dht.begin();
  
  // Light sensor
  pinMode(lightPin, INPUT);
}

void loop() {
  //Doc nhiet do va do am tu DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11 sensor!");
    delay(1000);
    return;
  }
  
  //Doc do am dat tu sensor
  valueMois = analogRead(moisturePin);
  moisture = (100-((valueMois/4095.00)*100));

  // Read light level from sensor
  int lightLevel = 4095 - analogRead(lightPin);

  // Create JSON object
  StaticJsonDocument<128> json;
  json["temp"] = String(temperature);
  json["humi"] = String(humidity);
  json["mois"] = String(moisture);
  json["light"] = String(lightLevel);

  // Serialize JSON to a string
  String jsonString;
  serializeJson(json, jsonString);

  // Connect to the server
  if (client.connect("172.20.10.2", 80)) { 
    Serial.println("Connected to server");

    // Make a POST request
    client.println("POST /postsensor HTTP/1.1");
    client.println("Host: 172.20.10.2"); 
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println();
    client.println(jsonString);

    // Wait for the server to respond
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.println(line);
      }
    }

    // Disconnect from the server
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Failed to connect to server");
  }

  delay(5000); // Delay 10 seconds before sending the next request
}
