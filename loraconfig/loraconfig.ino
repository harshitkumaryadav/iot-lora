#include <SoftwareSerial.h>

// Pins: RX = 2 (Connects to LoRa TX), TX = 3 (Connects to LoRa RX via Level Shifter)
SoftwareSerial loraSerial(2, 3); 

// === CONFIGURATION SETTINGS ===
const String NODE_ADDRESS = "1";     // Set to "1" for Arduino A, "2" for Arduino B
const String NETWORK_ID   = "15";    // Must match the Pi 5 network ID
const String FREQUENCY    = "865000000"; // 915MHz 
// ==============================

void setup() {
  Serial.begin(9600);        // PC Serial Monitor
  loraSerial.begin(115200);   // RYLR998 default factory baud rate
  
  delay(2000); // Wait for modules to stabilize after power-up
  Serial.println("--- Starting One-Time LoRa Configuration ---");

  // 1. Test communication
  sendATCommand("AT");
  
  // 2. Set Network ID
  sendATCommand("AT+NETWORKID=" + NETWORK_ID);
  
  // 3. Set Unique Node Address
  sendATCommand("AT+ADDRESS=" + NODE_ADDRESS);
  
  // 4. Set Frequency Band
  sendATCommand("AT+BAND=" + FREQUENCY);


  sendATCommand("AT+IPR=9600");
  
  Serial.println("\n--- Configuration Complete! ---");
  Serial.print("This board is now saved as Node Address: ");
  Serial.println(NODE_ADDRESS);
  Serial.println("You can now upload your main latency/loop receiver sketch.");
}

void loop() {
  // Keeping loop empty as configuration only needs to run once in setup
}

void sendATCommand(String command) {
  Serial.print("Sending: ");
  Serial.println(command);
  
  loraSerial.print(command + "\r\n"); // RYLR998 expects carriage return + newline
  delay(500); // Give the module time to process and write to EEPROM
  
  // Print the module's response (+OK) to the Serial Monitor
  while (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
  }
  Serial.println("-----------------------------------");
}
