#include <SoftwareSerial.h>

// Arduino RX (Pin 2) -> RYLR998 TXD
// Arduino TX (Pin 3) -> RYLR998 RXD
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600);   
  
  delay(1000);
  Serial.println("--- RECEIVER CONFIGURATION START ---");

 
  loraSerial.println("AT");
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+ADDRESS=2"); 
  delay(500);
  printLoRaResponse();


  loraSerial.println("AT+NETWORKID=18");
  delay(500);
  printLoRaResponse();
  
  Serial.println("Receiver...");
}

void loop() {

  if (loraSerial.available()) {
    String incoming = loraSerial.readString();
    
    Serial.println("\n------------------------------");
    Serial.print("LoRa: ");
    Serial.print(incoming); 
  
    Serial.println("------------------------------");
  }
}

void printLoRaResponse() {
  while (loraSerial.available()) {
    String resp = loraSerial.readString();
    Serial.print("Modem: ");
    Serial.print(resp);
  }
}