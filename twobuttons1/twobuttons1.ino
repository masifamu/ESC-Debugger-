#include <avr/io.h>
#include <EEPROM.h>
#include <Wire.h>
#include "OneButton.h"

#define EXTERNAL_MEM_SIZE 2048

// Setup a new OneButton on pin A1.  
OneButton mode(11, true);

const byte DEVADDR = 0x50;

int sensor_pin = 2; // IR sensor connection
unsigned int count=0;
unsigned int rpm=0; // RPM value
uint32_t timeBase=0,elapsedTime=0,prevTime=0, timePresent=0,totalTime=0,lastTotalTime=0;

float distance=0.0f,totalPower=0.0f;
float wheel_dia=0.6096f;//in meters

bool addressOverflown=false;
bool modeButtonSingleClicked=false;
bool modeButtonDoubleClicked=false;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Trip Analysis Module");

  // link the button 1 functions.
  mode.attachClick(click1);
  mode.attachDoubleClick(doubleclick1);

  startTimer1();
  pinMode(13, OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  turnGreenLEDOFF();
  
  pinMode(sensor_pin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor_pin), counter, FALLING);
}

// main code here, to run repeatedly: 
void loop() {
  static uint8_t batt3cellVolt=0,current=0,batt48Volt=0;
  static float temp=0.0f;
  
  if(modeButtonSingleClicked == true){
    turnGreenLEDON();
    
    batt3cellVolt=(uint8_t)(((analogRead(A0)*5.0f)/1024.0f)*((1.5+1)/1));
    batt48Volt=(uint8_t)(((analogRead(A1)*5.0f)/1024.0f)*((10+1)/1));
    temp = (((analogRead(A2)*5.0f)/1024.0f)-2.51f);
    current= temp < 0?(uint8_t)0:(uint8_t)(temp*15.15151515f);

    // Measure Actual RPM
    static uint32_t filterElapsedTime;
    filterElapsedTime = ((filterElapsedTime<<3)-filterElapsedTime+elapsedTime)>>3;
    rpm = (filterElapsedTime > 10)? (60000/filterElapsedTime):0;

    totalPower += (filterElapsedTime*batt48Volt*current)/60.0f/60.0f/1000.0f;//to get WH

    totalTime = timeBase/100;//in sec
    
    if(totalTime % 2 == 0 && lastTotalTime != totalTime && totalTime > 10){//after 10sec starts storing data
      lastTotalTime = totalTime;
      if(!addressOverflown){
        //storing data to internal EEPROM
        static int address;
        storeInt(address,rpm); address = address + 2;
        storeInt(address,batt48Volt); address = address + 2;
        storeInt(address,current); address = address + 2;
        storeLastAdd(address);
        
        //checking if the EEPROM is full: if full start storing into external eeprom
        if (address >= 1001){ addressOverflown = true; }
        Serial.println(address);

        storeLastExtEEPROMAdd(0);//to make sure the last add in EEPROM is zero 
        //if we are reading by only storing some samples in internal then the external eeprom add
        //stored previously will be invoked while reading and that is not desired
      }else{
        //store data to external eeprom
        static int extEEPROMAdd;
        storeIntToEEPROM(DEVADDR,extEEPROMAdd,rpm); extEEPROMAdd = extEEPROMAdd + 2;
        storeIntToEEPROM(DEVADDR,extEEPROMAdd,batt48Volt); extEEPROMAdd = extEEPROMAdd + 2;
        storeIntToEEPROM(DEVADDR,extEEPROMAdd,current); extEEPROMAdd = extEEPROMAdd + 2;

        //check if the external memory is full
        if(extEEPROMAdd <= EXTERNAL_MEM_SIZE) storeLastExtEEPROMAdd(extEEPROMAdd);
        else {
          modeButtonSingleClicked = false;
          modeButtonDoubleClicked = false;
          turnGreenLEDOFF();
        }
        Serial.println(extEEPROMAdd);
      }
    }
    //store distance, time, totalP
    storeInt(1001,(int)distance);
    storeInt(1003,totalTime);
    storeInt(1005,(int)totalPower);
    
  }else if(modeButtonDoubleClicked == true){
    turnREDLEDON();
    //reading EEPROM values
    static int add;
    while(add < readLastAdd()){
      Serial.print(readInt(add));Serial.print(" ");
      add += 2;
      if(add %6 == 0) Serial.println(" ");
    }
    if(add == readLastAdd()){
      //now read from external eeprom
      static int extEEAdd;
      while(extEEAdd < readLastExtEEPROMAdd()){
        Serial.print(readIntFromEEPROM(DEVADDR,extEEAdd));Serial.print(" ");
        extEEAdd += 2;
        if(extEEAdd %6 == 0) Serial.println(" ");
      }
      //now from internal eeprom.
      Serial.print("Distance: ");Serial.print(readInt(1001));Serial.println(" m");
      Serial.print("Time: ");Serial.print(readInt(1003));Serial.println(" s");
      Serial.print("totalPower: ");Serial.print(readInt(1005));Serial.println(" wh");
      
      add += 2;//to make sure this block executes only once.
      turnREDLEDOFF();
      while(1) blinkGreenLEDAt(100);
    }
  }

  delay(10);
} // loop






void counter() {
  count++;
  PORTD ^= (1<<PORTD3);
  
  timePresent = millis();
  elapsedTime = timePresent-prevTime;
  prevTime = timePresent;

  distance=distance+3.14f*wheel_dia;
}




void click1() {
  //Serial.println("Button 1 clicked.");
  if(modeButtonSingleClicked == true){
    modeButtonSingleClicked = false;
  }else{
    modeButtonSingleClicked = true;
    modeButtonDoubleClicked = false;
  }
} // click1
// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  //Serial.println("Button 1 doubleclick.");
  if(modeButtonDoubleClicked == true){
    modeButtonDoubleClicked = false;
  }else{
    modeButtonDoubleClicked = true;
    modeButtonSingleClicked = false;
  }
} // doubleclick1





void startTimer1(){
  //set timer1 interrupt at 100Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 100hz increments
  OCR1A = 20000;// = (16*10^6) / (100*8) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS11);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}
ISR(TIMER1_COMPA_vect){//timer1 interrupt 100Hz toggles
  timeBase++;//one increment in timeBase is equal to 10ms.
  mode.tick();
}







void blinkGreenLEDAt(unsigned int msDelayT){ PORTD ^= (1<<PORTD4); delay(msDelayT); }
void turnGreenLEDON(){ PORTD |= (1<<PORTD4); }
void turnGreenLEDOFF(){ PORTD &= ~(1<<PORTD4); }
void turnREDLEDON(){ PORTD |= (1<<PORTD3); }
void turnREDLEDOFF(){ PORTD &= ~(1<<PORTD3); }






void storeLastAdd(int addVal){
  int q,r;
  q=addVal/256;
  r=addVal%256;
  EEPROM.update(1007, q);
  EEPROM.update(1008, r);
}
int readLastAdd(){ return (EEPROM.read(1007)*256+EEPROM.read(1008)); }







void storeLastExtEEPROMAdd(int addVal){
  int q,r;
  q=addVal/256;
  r=addVal%256;
  EEPROM.update(1009, q);
  EEPROM.update(1010, r);
}
int readLastExtEEPROMAdd(){ return (EEPROM.read(1009)*256+EEPROM.read(1010)); }







void storeInt(int baseAdd,int integer){
  unsigned int q=0,r=0;
  q=integer/256;
  r=integer%256;
  EEPROM.update(baseAdd, q);
  EEPROM.update(baseAdd+1, r);
}
int readInt(int baseAdd){
  return (EEPROM.read(baseAdd)*256+EEPROM.read(baseAdd+1));
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





// End
