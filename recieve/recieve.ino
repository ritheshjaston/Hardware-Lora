#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define ss 5
#define rst 14
#define dio0 2

// leds
#define red 0
#define green 4

// Shared variables
volatile int found = 0;  // Use volatile for shared variables between ISR and main code
String LData = "";

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// HTTP Server details
const char* serverAddress = "http://yourserver.com/endpoint";

// Task handle
TaskHandle_t Task1;

void setup() {
  Serial.begin(115200);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);

  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  LoRa.setPins(ss, rst, dio0);  // Setup LoRa transceiver module

  while (!LoRa.begin(433E6)) {  // 433E6 - Asia, 866E6 - Europe, 915E6 - North America
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
  digitalWrite(green, HIGH);
  delay(100);
  digitalWrite(green, LOW);
  delay(100);
  digitalWrite(red, HIGH);
  delay(100);
  digitalWrite(red, LOW);

  // Initialize your task (2nd loop)
  xTaskCreatePinnedToCore(
    loop2,          // name of the task function
    "buttonCheck",  // name of the task
    10000,          // memory assigned for the task
    NULL,           // parameter to pass if any
    1,              // priority of task, lower priority than main loop
    &Task1,         // Reference name of taskHandle variable
    0);             // choose core (0 or 1)
}

void loop() {
  int packetSize = LoRa.parsePacket();  // Try to parse packet
  if (packetSize) {
    Serial.print("Received packet '");
    while (LoRa.available()) {  // Read packet
      String LoRaData = LoRa.readString();
      found = 1;
      Serial.print(LoRaData + "\n");
      LData = "data=" + String(LoRaData);
    }
    Serial.print("' with RSSI ");  // Print RSSI of packet
    Serial.println(LoRa.packetRssi());
  }
}

void loop2(void* parameter) {
  while (1) {
    if (found == 1) {
      digitalWrite(green, LOW);
      digitalWrite(red, HIGH);
      delay(1000);
      digitalWrite(red, LOW);

      // Perform HTTP request to send LData
      HTTPClient http;
      http.begin(serverAddress);                                            // Specify the server address
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Specify content type
      int httpResponseCode = http.POST(LData);                              // Send LData in the request

      // Check for errors
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
      } else {
        Serial.print("Error during HTTP request: ");
        Serial.println(httpResponseCode);
      }

      http.end();  // Close connection

      found = 0;  // Reset found flag after processing
    } else {
      digitalWrite(green, HIGH);
    }
    vTaskDelay(100);  // Delay to avoid excessive task execution
  }
}
