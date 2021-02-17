/*
Arduino uno reading eeprom 24c16
pins    Arduino     24c16
        5V          VCC
        GND         GND
        SCL         SCL
        SDA         SDA
Dont use analog pin A0,or raplace it in code to free analog pin        
*/
#include <Wire.h>
const byte DEVADDR = 0x50;
byte msgf = 0x42;
   
void setup() 
{
   Wire.begin();
   Serial.begin(115200);

   for(uint16_t i=0;i<2048;i++){
            byte b = eeprom_read_byte(DEVADDR, i);
            if(i%16 == 0) Serial.println(" ");
            Serial.print(b);Serial.print(" ");
          }
}

void loop() 
{   
          storeIntToEEPROM(DEVADDR,10,252);
          int b =readIntFromEEPROM(DEVADDR,10);


          Serial.println(b);
          delay(10);

          
}
void eeprom_write_page(byte deviceaddress, unsigned eeaddr,
                      byte data)
{
   // Three lsb of Device address byte are bits 8-10 of eeaddress
   byte devaddr = deviceaddress | ((eeaddr >> 8) & 0x07);
   int addr    = eeaddr;
   //Serial.println(addr);
   Wire.beginTransmission(devaddr);
   Wire.write(int(addr));
   Wire.write(data);
   Wire.endTransmission();
   delay(10);
}
void storeIntToEEPROM(byte chipAdd, unsigned memAdd,unsigned int data){
  byte q = (byte)(data/256);
  byte r = (byte)(data%256);
  eeprom_write_page(chipAdd, memAdd,q);
  eeprom_write_page(chipAdd, memAdd+1,r);
}
int eeprom_read_byte(byte deviceaddress, unsigned eeaddr)
{
   byte rdata = -1;

   // Three lsb of Device address byte are bits 8-10 of eeaddress
   byte devaddr = deviceaddress | ((eeaddr >> 8) & 0x07);
   byte addr    = eeaddr;

   Wire.beginTransmission(devaddr);
   Wire.write(int(addr));
   Wire.endTransmission();
   Wire.requestFrom(int(devaddr), 1);
   if (Wire.available()) {
       rdata = Wire.read();
   }
   return rdata;
}
int readIntFromEEPROM(byte chipAdd, unsigned memAdd){
  return((int)eeprom_read_byte(chipAdd, memAdd)*256+(int)eeprom_read_byte(chipAdd, memAdd+1));
}
