#include <SoftwareSerial.h>

// RX -> D2 (RYLR TX), TX -> D3 (RYLR RX)
SoftwareSerial rylrSerial(2, 3); 

String nodeID = "1";       
byte sequenceNumber = 0;   

void setup() {
  Serial.begin(9600);
  rylrSerial.begin(9600); 
  
  delay(1000);
  Serial.println("RYLR998 Node 1 Initializing...");
  
  
  rylrSerial.println("AT+ADDRESS=" + nodeID);
  delay(500);
}

void loop() {
  
  String fixedData = "1,2,3,4,5,6,7,8,9";
  
  // 1.(NodeID,SequenceNumber,Data)
  String partialPacket = nodeID + "," + String(sequenceNumber) + "," + fixedData;

  // 2. with checksum
  unsigned int totalSum = 0;
  for (int i = 0; i < partialPacket.length(); i++) {
    totalSum += partialPacket[i]; 
  }
  byte normalChecksum = totalSum % 256;

  String finalPayload = partialPacket + "," + String(normalChecksum);


  String atCommand = "AT+SEND=0," + String(finalPayload.length()) + "," + finalPayload;
  rylrSerial.println(atCommand);

  Serial.print("Sent: "); Serial.println(atCommand);

  sequenceNumber++; //(0-255)
  
  // 1 sec+random
  int randomOffset = random(0, 200);
  delay(1000 + randomOffset);
//delay(1000);
}
