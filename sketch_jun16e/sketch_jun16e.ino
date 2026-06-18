#include <SoftwareSerial.h>

// RX pin 2 (Connect to LoRa TX), TX pin 3 (Connect to LoRa RX)
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600); // Assumes your receiver RYLR998 is also running at 9600 baud
  
  delay(2000); // Give the LoRa hardware time to fully initialize
  Serial.println("--- RECEIVER NODE READY ---");
  Serial.println("Listening for incoming transmissions...");
  Serial.println("------------------------------------------------");
}

void loop() {
  // 1. READ INCOMING RESPONSES / MESSAGES FROM LORA
  if (loraSerial.available()) {
    delay(30); // Gives the serial buffer a small window to fully capture the incoming packet
    
    Serial.print("[Incoming Data]: ");
    while (loraSerial.available()) {
      char c = loraSerial.read(); 
      Serial.print(c);
    }
    // Note: No extra Serial.println() here because the RYLR998 output automatically ends with \r\n
  }

  // 2. READ FROM SERIAL MONITOR (For configuring or replying back)
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  

    if (input.length() > 0) {
      String commandToExecute = "";

      // Create a temporary string to check for AT command intent
      String checkInput = input;
      checkInput.toUpperCase(); 

      if (checkInput.startsWith("AT")) {
        // Enforce full uppercase for raw AT commands to prevent +ERR=2
        input.toUpperCase(); 
        commandToExecute = input + "\r\n"; 
        Serial.print("[Command Sent]: ");
        Serial.println(input);
      } 
      else {
        // Auto-wrap normal text to target Sender Node (Address 1)
        commandToExecute = "AT+SEND=1," + String(input.length()) + "," + input + "\r\n";
        Serial.print("[Reply Sent]: ");
        Serial.println(commandToExecute);
      }

      // Flush out any background buffer clutter before pushing the data
      while(loraSerial.available() > 0) { loraSerial.read(); }

      // Transmit packet to the RYLR998
      loraSerial.print(commandToExecute);
    }
  }
}
