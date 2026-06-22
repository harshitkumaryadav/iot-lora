#include <SoftwareSerial.h>

// RX -> D2 (RYLR TX), TX -> D3 (RYLR RX)
SoftwareSerial rylrSerial(2, 3); 

String nodeID = "1";      
byte sequenceNumber = 0;  
String currentSyncedTime = "00:00:00.000"; 
bool isTimeSynced = false;

void setup() {
  Serial.begin(9600);
  rylrSerial.begin(9600); 
  
  delay(1000);
  Serial.println("RYLR998 Node 1 Initializing...");
  
  rylrSerial.println("AT+ADDRESS=" + nodeID);
  delay(500);
  
  delay(500);

   Serial.println("requesting time from pi ");

   while(!isTimeSynced){
    String reqCommand = "AT+SEND=0,8,REQ_TIME";
    rylrSerial.println(reqCommand);
    Serial.println("Sent : REQ_TIMe. waiting for response....");

    unsigned long startWait = millis();
    while (millis()-startWait<3000){
      if (rylrSerial.available() > 0){
        String incoming =rylrSerial.readStringUntil('\n');

        if (incoming.startsWith("+RCV=")){
          int fcoma=incoming.indexOf(',');
          int scoma=incoming.indexOf(',' , fcoma+1);
          int tcoma=incoming.indexOf(',' , scoma+1);
          int focoma=incoming.indexOf(',' , tcoma+1);
//          int firstdot=incoming.indexOf('.');
          String payload=incoming.substring(focoma);

          if(payload.startsWith("TIME:")){
            currentSyncedTime = payload.substring(5);
            isTimeSynced = true;
            Serial.print("success time synced");
            Serial.println(currentSyncedTime);
            break;
          }

        }
      }
      }
    if (!isTimeSynced){
      Serial.println("no response from pi ,retry in 2 seconds...");
      delay(2000);
      }
    }
}

void loop() {
  String fixedData = "1,2,3,4,5,6,7,8,9";
  
 
  String partialPacket = nodeID + "," + String(sequenceNumber) + "," + fixedData;

  unsigned int totalSum = 0;
  for (int i = 0; i < partialPacket.length(); i++) {
    totalSum += partialPacket[i]; 
  }
  byte normalChecksum = totalSum % 256;

  
  String finalPayload = partialPacket + "," + String(normalChecksum);

  
  String atCommand = "AT+SEND=0," + String(finalPayload.length()) + "," + finalPayload;
  rylrSerial.println(atCommand);

  Serial.print("Sent: "); Serial.println(atCommand);

  sequenceNumber++; 
  
  int randomOffset = random(0, 200);
  delay(1000 + randomOffset);
}
