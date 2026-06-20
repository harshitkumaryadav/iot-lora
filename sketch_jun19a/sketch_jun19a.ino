#include <SoftwareSerial.h>

SoftwareSerial loraSerial(2, 3); // RX, TX (Ensure you are using a 3.3V logic shifter on Pin 3!)

void setup() {
  Serial.begin(9600);        // Hardware Serial to your PC
  loraSerial.begin(9600);   // RYLR998 default speed
  Serial.println("Arduino Loop Listener Ready...");
}

void loop() {
  if (loraSerial.available()) {
    String receivedRaw = loraSerial.readStringUntil('\n');
    
    if (receivedRaw.startsWith("+RCV")) {
      // Break down the received buffer (+RCV=<Address>,<Length>,<Payload>,...)
      String dataString = receivedRaw.substring(5);
      
      int firstComma = dataString.indexOf(',');
      int secondComma = dataString.indexOf(',', firstComma + 1);
      int thirdComma = dataString.indexOf(',', secondComma + 1);
      
      String payload = dataString.substring(secondComma + 1, thirdComma);
      
      // Output directly to your screen
      Serial.print("Loop Stream Packet: ");
      Serial.println(payload);
      
      // Echo back immediately to Pi 5 (Address 0)
      delay(5); // Ultra-short buffer stabilization delay
      String echoCommand = "AT+SEND=0," + String(payload.length()) + "," + payload + "\r\n";
      loraSerial.print(echoCommand);
    }
  }
}
