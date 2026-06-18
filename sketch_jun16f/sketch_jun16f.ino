#include <SoftwareSerial.h>

// RX pin 2 (Connect to LoRa TX), TX pin 3 (Connect to LoRa RX)
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600); 
  
  delay(1000);

  // Force the module to assign itself Address 2 on boot
  loraSerial.print("AT+ADDRESS=1\r\n"); 
  delay(500);
  
  // Ensure it matches the sender network group
  loraSerial.print("AT+NETWORKID=18\r\n");
  delay(500);
  Serial.println("--- SYSTEM READY ---");
  //Serial.println("-> Type 'AT...' to send a direct raw command.");
  //Serial.println("-> Type any other text to broadcast it via LoRa to Address 2.");
  Serial.println("------------------------------------------------");
}

void loop() {
  // 1. READ FROM SERIAL MONITOR
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
   
    input.trim(); 

    input.toUpperCase();
    
    if (input.length() > 0) {
      String commandToExecute = "";

      // Check if you typed a raw AT command
      if (input.startsWith("AT") || input.startsWith("at")) {
        commandToExecute = input + "\r\n"; 
        Serial.print("[Command Sent]: ");
        Serial.println(input);
      } 
      else {
        // It's a normal  -> Auto-wrap it
        commandToExecute = "AT+SEND=2," + String(input.length()) + "," + input + "\r\n";
        Serial.print("[Message Sent]: ");
        Serial.println(commandToExecute);
      }

      // Flush out any old background noise in the LoRa buffer before sending
      while(loraSerial.available() > 0) { loraSerial.read(); }

      // Transmit the clean command to the RYLR998
      loraSerial.print(commandToExecute);
    }
  }

  // 2. READ INCOMING RESPONSES FROM LORA
  if (loraSerial.available()) {
    delay(30); // Give the buffer time to fill completely
    Serial.print("Modem Response: ");
    while (loraSerial.available()) {
      char c = loraSerial.read(); 
      Serial.print(c);
    }
  }
}
