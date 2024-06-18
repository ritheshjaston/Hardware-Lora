#include <LoRa.h>
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>     // Adafruit  sensor library
#include <Adafruit_ADXL345_U.h>  // ADXL345 library


#define ss 5
#define rst 14
#define dio0 2

#define fire 4

const int buzzerPin = 13;  // Buzzer pin


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();  // ADXL345 Object
sensors_event_t event;
float prev_x, prev_y, prev_z;

int counter = 0;

const int threshold = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);    // Set buzzer pin as output
  digitalWrite(buzzerPin, LOW);  // Make sure buzzer is initially off
  pinMode(fire, INPUT);          // Set the heat sensor pin as input
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  LoRa.setPins(ss, rst, dio0);  //setup LoRa transceiver module

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");

  //accelerometer
  if (!accel.begin())  // if ASXL345 sensor not found
  {
    Serial.println("ADXL345 not detected");
    while (1)
      ;
  } else {
    accel.setRange(ADXL345_RANGE_16_G);
  }

  accel.getEvent(&event);
  prev_x = event.acceleration.x;
  prev_y = event.acceleration.y;
  prev_z = event.acceleration.z;
}

void loop() {

  accel.getEvent(&event);

  // check if current acceleration is a sudden change from previous acceleration
  float dx = event.acceleration.x - prev_x;
  float dy = event.acceleration.y - prev_y;
  float dz = event.acceleration.z - prev_z;
  if ((dx * dx) + (dy * dy) + (dz * dz) > 400) {
    // threshold for sudden change in acceleration
    Serial.print("ALERT: Sudden change in acceleration detected! X: ");
    Serial.print(event.acceleration.x);
    Serial.print(" Y: ");
    Serial.print(event.acceleration.y);
    Serial.print(" Z: ");
    Serial.println(event.acceleration.z);
    Serial.print("Sending packet: ");
    Serial.println(counter);

    LoRa.beginPacket();  //Send LoRa packet to receiver
    LoRa.print("ACC KA19HE1567 76.00 72.90");
    // LoRa.print(counter);
    LoRa.endPacket();

    counter++;

    delay(1000);
    while (1) {
      if (touchRead(12) < 40) {
        LoRa.beginPacket();  //Send LoRa packet to receiver
        LoRa.print("STOP KA19HE1567");
        // LoRa.print(counter);
        LoRa.endPacket();
        digitalWrite(buzzerPin, LOW);
        break;
      }else{
        digitalWrite(buzzerPin, HIGH);
        // delay(1000)
        // digitalWrite(buzzerPin, LOW);
        // delay(1000)
      }
    }
  }
  int heatValue = digitalRead(fire);  // Read digital value from sensor

  if (heatValue == threshold) {
    // Fire detected, trigger alert mechanism here
    Serial.println("Fire detected!");
    Serial.print("Sending packet: ");
    Serial.println(counter);

    LoRa.beginPacket();  //Send LoRa packet to receiver
    LoRa.print("FIRE KA19HE1567 76.00 72.90");
    // LoRa.print(counter);
    LoRa.endPacket();

    counter++;

    delay(1000);
    while (1) {
      if (touchRead(12) < 40) {
        LoRa.beginPacket();  //Send LoRa packet to receiver
        LoRa.print("STOP KA19HE1567");
        // LoRa.print(counter);
        LoRa.endPacket();
        digitalWrite(buzzerPin, LOW);
        break;
      }else{
        digitalWrite(buzzerPin, HIGH);
        // delay(1000)
        // digitalWrite(buzzerPin, LOW);
        // delay(1000)
      }
    }
  }
}