#include <Wire.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
SoftwareSerial lora(2, 3); // first argument is rx pin and second tx pin
// (pin 2 - rx, pin 3 - tx)micro ---> (tx,rx)rylr998
const int LED = 13;
String data ;
void setup()
{
Serial.begin(115200);
lora.begin(115200);
pinMode(LED, OUTPUT);
}
void loop()
{
String inString;
while (lora.available())
{
if (lora.available()) {
inString += String(char(lora.read()));
}
}
if (inString.length() > 0)
{
Serial.println(inString);
String inString1 = inString.substring(inString.indexOf(",") + 1);
String inString2 = inString1.substring(inString1.indexOf(",") + 1);
data = inString2.substring(0, inString2.indexOf(","));
Serial.println(data);
analogWrite(LED, data.toInt());
delay(2);
}
data = "";
}
