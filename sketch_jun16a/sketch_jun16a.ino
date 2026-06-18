#include <SoftwareSerial.h>

SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(115200);   
  
  delay(1000);
  Serial.println("--- SENDER CONFIGURATION START ---");

  loraSerial.println("AT+IPR=9600"); 
  delay(500);
  printLoRaResponse();
  
  loraSerial.println("AT");
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+ADDRESS=1"); 
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+NETWORKID=18");
  delay(500);
  printLoRaResponse();
  
  Serial.println("--- CONFIGURATION END ---");
}

void loop() {
  // 1. Read from Serial Monitor and send to LoRa
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim(); 
    
    if (msg.length() > 0) {
      String command = "AT+SEND=2," + String(msg.length()) + "," + msg;
      loraSerial.println(command);
      
      Serial.print("Command sent: ");
      Serial.println(command);
    }
  }

  // 2. Read from LoRa and print to Serial Monitor character-by-character
  if (loraSerial.available()) {
    Serial.print("Modem Response: ");
    while (loraSerial.available()) {
      char c = loraSerial.read(); // Read one character at a time
      Serial.print(c);
      delay(2); // Small delay to let the next buffer byte arrive
    }
    Serial.println(); // New line after response ends
  }
}

void printLoRaResponse() {
  // Prevent freezing by checking if data actually exists
  if (loraSerial.available()) {
    Serial.print("Modem: ");
    while (loraSerial.available()) {
      char c = loraSerial.read(); // Read one character at a time
      Serial.print(c);
      delay(2);
    }
    Serial.println();
  }
}
