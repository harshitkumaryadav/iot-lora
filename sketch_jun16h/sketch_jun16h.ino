#include <SoftwareSerial.h>

// 0RX pin 2 (Connect to LoRa TX), TX pin 3 (Connect to LoRa RX)
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600); // Set to 9600 for stable SoftwareSerial communication
  
  delay(2000); // Give the LoRa hardware time to fully initialize
  Serial.println("--- RECEIVER SYSTEM ONLINE ---");
  Serial.println("-> Ready to receive wireless data from air.");
  Serial.println("-> Type any 'AT' command to configure this receiver module.");
  Serial.println("----------------------------------------------------------");
}

void loop() {
  // 1. READ INCOMING RESPONSES / WIRELESS MESSAGES FROM LORA
  if (loraSerial.available() > 0) {
    // Label the data coming out of the LoRa chip
    Serial.print("LoRa: ");
    
    // Read and print everything directly from the module buffer instantly
    while (loraSerial.available() > 0) {
      char c = loraSerial.read(); 
      Serial.print(c);
    }
  }

  // 2. READ FROM SERIAL MONITOR (For local AT commands or text replies)
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  

    if (input.length() > 0) {
      String commandToExecute = "";

      // Create a temporary string to check for an AT command
      String checkInput = input;
      checkInput.toUpperCase(); 

      if (checkInput.startsWith("AT")) {
        // Enforce full uppercase for raw AT commands to prevent +ERR=2
        input.toUpperCase(); 
        commandToExecute = input + "\r\n"; 
        Serial.print("[Command Sent Local]: ");
        Serial.println(input);
      } 
      else {
        // Auto-wrap normal text as a reply to target Sender Node (Address 1)
        commandToExecute = "AT+SEND=1," + String(input.length()) + "," + input + "\r\n";
        Serial.print("[Wireless Message Sent]: ");
        Serial.println(commandToExecute);
      }

      // Flush out any background buffer noise before pushing data to the module
      while(loraSerial.available() > 0) { loraSerial.read(); }

      // Transmit the clean packet to the RYLR998
      loraSerial.print(commandToExecute);
    }
  }
}
