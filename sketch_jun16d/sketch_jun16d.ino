#include <SoftwareSerial.h>

// RX pin 2 (Connect to LoRa TX), TX pin 3 (Connect to LoRa RX)
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(9600); 
  
  delay(1000);
  Serial.println("--- SYSTEM READY ---");
  Serial.println("-> Type 'AT...' to send a direct raw command.");
  Serial.println("-> Type any other text to broadcast it via LoRa to Address 2.");
  Serial.println("------------------------------------------------");
}

void loop() {
  // 1. READ FROM SERIAL MONITOR
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Cleans up hidden whitespace or carriage returns
    
    if (input.length() > 0) {
      String commandToExcute = "";

      // Check if the input is a raw AT command
      if (input.startsWith("AT") || input.startsWith("at")) {
        // Option A: Pass the raw AT command straight through
        commandToExcute = input + "\r\n";
        Serial.print("[Direct Command]: ");
        Serial.print(commandToExcute);
      } 
      else {
        // Option B: It's a normal message! Auto-wrap it into a LoRa SEND command
        commandToExcute = "AT+SEND=2," + String(input.length()) + "," + input + "\r\n";
        Serial.print("[Auto-Wrapped Send]: ");
        Serial.print(commandToExcute);
      }

      // Transmit the final formatted string to the RYLR998
      loraSerial.print(commandToExcute);
    }
  }

  // 2. READ INCOMING RESPONSES FROM LORA
  if (loraSerial.available()) {
    // Give the serial buffer a tiny window to completely capture the incoming string
    delay(30); 
    Serial.print("Modem Response: ");
    while (loraSerial.available()) {
      char c = loraSerial.read(); 
      Serial.print(c);
    }
    // No extra println here because the RYLR998 already ends its output with \r\n
  }
}
