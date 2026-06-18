#include <SoftwareSerial.h>

SoftwareSerial loraSerial(2, 3); 

void setup() {
  Serial.begin(9600);       
  loraSerial.begin(115200); // Talk to it at factory default one last time  
  
  delay(1000);
  Serial.println("Sending baud rate change command...");

  loraSerial.println("AT+IPR=9600"); // Module switches to 9600 instantly
  delay(500);
  
  // Print response (it might look slightly garbled as the switch happens mid-transmission)
  while (loraSerial.available()) {
    Serial.print((char)loraSerial.read());
  }
  Serial.println("\nBaud rate updated! Now upload your main production code.");
}

void loop() {
}
