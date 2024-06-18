#include <HardwareSerial.h>
HardwareSerial Sim800L(2);

void setup() {
  Serial.begin(115200);
  Serial.println("Goodnight moon!");
  Sim800L.begin(115200);
  Sim800L.println("AT");
  delay(500);
  UpdateSerial();
  Sim800L.println("AT+GPS=1"); // Enable GPS
  delay(500);
  UpdateSerial();
}

void loop() {
  Sim800L.println("AT+LOCATION=2"); // Get location
  delay(500);
  UpdateSerial();
}

void parseData(String buff) {
  if (buff.indexOf(',') != -1) { // Check if location information is found

    Serial.println("Data Location :  "+buff);
   
  }else{
    Serial.println("ass "+buff);
  }
}

void UpdateSerial() {
  while (Sim800L.available()) {
    parseData(Sim800L.readStringUntil('\n')); // Parse data line by line
  }

  while (Serial.available()) {
    Sim800L.write(Serial.read());
  }
}
