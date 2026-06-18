 #include <SoftwareSerial.h>

SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600);   
  
  delay(1000);
  Serial.println("--- SENDER CONFIGURATION START ---");

  loraSerial.println("AT");
  delay(500);
  printLoRaResponse();

  
  loraSerial.println("AT+ADDRESS=1"); 
  delay(500);
  printLoRaResponse();

  loraSerial.println("AT+NETWORKID=18");
  delay(500);
  printLoRaResponse();
  
  Serial.println("Sender ");
}

void loop() {
  
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim(); 
    
    if (msg.length() > 0) {
      String command = "AT+SEND=2," + String(msg.length()) + "," + msg;
      loraSerial.println(command);
      
      Serial.print("command is send  ");
      Serial.println(command);
    }
  }

  if (loraSerial.available()) {
    String incoming = loraSerial.readString();
    Serial.print("Modem Response: ");
    Serial.print(incoming);
  }
}

void printLoRaResponse() {
  while (loraSerial.available()) {
    String resp = loraSerial.readString();
    Serial.print("Modem: ");
    Serial.print(resp);
  }
}







// Arduino RX (Pin 2) -> RYLR998 TXD
// Arduino TX (Pin 3) -> RYLR998 RXD
