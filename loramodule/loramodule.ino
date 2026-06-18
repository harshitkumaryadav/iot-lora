#include <SoftwareSerial.h>

// RX (Pin 2) -> RYLR998 TXD
// TX (Pin 3) -> RYLR998 RXD
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);      
  loraSerial.begin(9600);   
  
  delay(1000);
  Serial.println("loRa is now started");

  loraSerial.println("AT");
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+ADDRESS=1"); 
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+NETWORKID=18");
  delay(500);
  printLoRaResponse();
  
  Serial.println("all is well :> Now you can start typing ");

}

void loop() {
  //for sending
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    
   
    // AT+SEND=<Target Address>,<Length>,<Data>
    String command = "AT+SEND=2," + String(msg.length()) + "," + msg;
    loraSerial.println(command);
    
    Serial.print("command :");
    Serial.println(command);
  }

  if (loraSerial.available()) {
    String incoming = loraSerial.readString();
    Serial.print("LoRa data: ");
    Serial.println(incoming);
  }
}

void printLoRaResponse() {
  while (loraSerial.available()) {
    String resp = loraSerial.readString();
    Serial.print("Modem: ");
    Serial.println(resp);
  }
}
