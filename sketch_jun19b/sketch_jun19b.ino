#include <SoftwareSerial.h>

// Pins: RX = 2 (Connect to LoRa TX), TX = 3 (Connect to LoRa RX via Level Shifter)
SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);        // PC USB Serial Monitor
  loraSerial.begin(9600);    // Matched perfectly to the Pi module's speed
  Serial.println("Arduino Loop Listener Online at 9600 Baud...");
}

void loop() {
  if (loraSerial.available() > 0) {
    String receivedRaw = loraSerial.readStringUntil('\n');
    receivedRaw.trim(); // Strip formatting spaces or hidden characters

    Serial.print(receivedRaw);
    if (receivedRaw.startsWith("+RCV")) {
      // Incoming packet format: +RCV=<Address>,<Length>,<Payload Data>,<RSSI>,<SNR>
      String dataString = receivedRaw.substring(5); // Strip "+RCV="
      
      int firstComma  = dataString.indexOf(',');
      int secondComma = dataString.indexOf(',', firstComma + 1);
      int thirdComma  = dataString.indexOf(',', secondComma + 1);
      
      String payload = "";
      if (thirdComma == -1) {
        payload = dataString.substring(secondComma + 1);
      } else {
        payload = dataString.substring(secondComma + 1, thirdComma);
      }
      
      Serial.print("Received Frame -> Echoing back: ");
      Serial.println(payload);
      
      // Echo back directly to Pi 5 (Address 0)
      String echoCommand = "AT+SEND=0," + String(payload.length()) + "," + payload + "\r\n";
      loraSerial.print(echoCommand);
    }
  }
}
