#include <SoftwareSerial.h>
SoftwareSerial lora(2, 3);
String data ;
int d_length ;
int address = 2;
String cmd ;
int a = 0;
int flag = 0;
void setup()
{
Serial.begin(115200);
lora.begin(115200);
}
void loop()
{
if (flag == 0) {
a+=3;
if(a>=255){
flag=1;
a=255;
}
}
else if (flag == 1) {
a-=9;
if(a<=0){
flag=0;
a=0;
}
}
data = (String)a;
d_length = data.length();
cmd = "AT+SEND=" + (String)address + "," + (String)d_length + "," + data;
lora.println(cmd);
delay(6);
}
