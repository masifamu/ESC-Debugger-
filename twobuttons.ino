#include <avr/io.h>
#include <SPI.h>
#include <Adafruit_GFX.h>           //Includes core graphics library
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#include "OneButton.h"
// Setup a new OneButton on pin A1.  
OneButton mode(11, true);
// Setup a new OneButton on pin A2.  
OneButton select(12, true);

typedef enum{DebugESC, ConfigESC, LEDDelay}State;
State modeState = DebugESC;
unsigned int modeSingleClickCount=0;

typedef enum{MainMenu, DbgMenu, CfgMenu, LEDMenu}OLEDState;
OLEDState oledDisplayState = MainMenu;

//float batt3cellVolt=0;

int sensor_pin = 2; // IR sensor connection
int count=0;


const int dataIN = 2; //IR sensor INPUT

unsigned long prevmillis; // To store time
unsigned long duration; // To store time difference
int rpm; // RPM value

boolean currentstate; // Current state of IR input scan
boolean prevstate; // State of IR sensor in previous scan


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Starting TwoButtons...");

  // link the button 1 functions.
  mode.attachClick(click1);
  mode.attachDoubleClick(doubleclick1);

  // link the button 2 functions.
  select.attachClick(click2);
  select.attachDoubleClick(doubleclick2);



  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  sendOLEDWelcomeText();

  startTimer1();
  pinMode(13, OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  
  pinMode(dataIN,INPUT); 
  prevmillis = 0;
  prevstate = LOW;
  
} // setup


// main code here, to run repeatedly: 
void loop() {
  static uint8_t batt3cellVolt=0,current=0,batt48Volt=0;
  static float temp=0.0f;
  
  // You can implement other code in here or just wait a while 
  if(oledDisplayState == MainMenu){
    OLEDMenu();
  }else if(oledDisplayState == DbgMenu){
    batt3cellVolt=(uint8_t)(((analogRead(A0)*5.0f)/1024.0f)*((1.5+1)/1));
    batt48Volt=(uint8_t)(((analogRead(A1)*5.0f)/1024.0f)*((10+1)/1));
    temp = (((analogRead(A2)*5.0f)/1024.0f)-2.51f);
    current= temp < 0?(uint8_t)0:(uint8_t)(temp*15.15151515f);

     // Measure Actual RPM 

  Serial.println(rpm);
  
    dbgMenu(batt3cellVolt,batt48Volt,current,rpm);
  }else if(oledDisplayState == CfgMenu){
    cfgMenu();
  }else if(oledDisplayState == LEDMenu){
    ledMenu(20);
    blinkLEDs();
  }
  
  delay(10);
} // loop


// ----- button 1 callback functions

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1() {
  //Serial.println("Button 1 click.");
  if(++modeSingleClickCount > 2) modeSingleClickCount=0;
  if(modeSingleClickCount == 0){
    modeState = DebugESC;
  }else if(modeSingleClickCount==1){
    modeState = ConfigESC;
  }else if(modeSingleClickCount == 2){
    modeState = LEDDelay;
  }
} // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("Button 1 doubleclick.");
} // doubleclick1


// ... and the same for button 2:

void click2() {
  //Serial.println("Button 2 click.");
  if(modeState == DebugESC) oledDisplayState = DbgMenu;
  else if(modeState == ConfigESC) oledDisplayState = CfgMenu;
  else if(modeState == LEDDelay) oledDisplayState = LEDMenu;
  else oledDisplayState = MainMenu;
} // click2


void doubleclick2() {
  //Serial.println("Button 2 doubleclick.");
  oledDisplayState = MainMenu;
} // doubleclick2

void sendOLEDWelcomeText(){
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F(" Punchline"));
  display.println(F("  Energy"));
  display.println(F(" Pvt. Ltd."));
  display.display();
  delay(1000);
}

void OLEDMenu(){
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("0:Dbg ESC"));
  display.println(F("1:Cfg ESC"));
  display.println(F("2:LEDs"));
  display.setCursor(0,50);
  display.println(F("M:"));
  display.setCursor(26,50);
  if(modeState == DebugESC) display.println(F("0"));
  else if(modeState == ConfigESC) display.println(F("1"));
  else if(modeState == LEDDelay) display.println(F("2"));
  display.setCursor(90,50);
  display.println(F("SEL"));
  display.display();
}
void dbgMenu(uint8_t v12, uint8_t v48,uint8_t curr, uint16_t rpm){
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(BLACK,WHITE);        // Draw white text
  display.setCursor(20,0);             // Start at top-left corner
  display.println(F("**Debug Menu**"));

  //making first row
  display.setTextColor(WHITE);
  display.setCursor(0,10); 
  display.setTextSize(2); 
  display.print(v12);
  display.println(F("V"));//battery capacity

  display.setCursor(64,10);
  display.print(rpm);

  //making second row
  //display.setTextColor(WHITE);
  display.setCursor(0,30); 
  display.setTextSize(2); 
  display.print(v48);
  display.println(F("V"));//battery capacity

  display.setCursor(64,30);
  display.print(curr);
  display.println(F("A"));//battery capacity
/*
  //making third row
  //display.setTextColor(WHITE);
  display.setCursor(0,50); 
  display.setTextSize(2); 
  display.print(rpm);
  display.println(F("v"));//battery capacity

  display.setCursor(64,50);
  display.print(rpm);
  display.println(F("v"));//battery capacity
  */
  display.display();
}
void cfgMenu(){
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(BLACK,WHITE);        // Draw white text
  display.setCursor(25,0);             // Start at top-left corner
  display.println(F("**Cfg Menu**"));
  display.display();
}

void ledMenu(unsigned int delayTime){
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(BLACK,WHITE);        // Draw white text
  display.setCursor(25,0);             // Start at top-left corner
  display.println(F("**LED Menu**"));
  display.display();
}






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
  // keep watching the push buttons:
  mode.tick();
  select.tick();

   currentstate = digitalRead(dataIN); // Read IR sensor state
 if( prevstate != currentstate) // If there is change in input
   {
     if( currentstate == HIGH ) // If input changes only from LOW to HIGH
       {
         duration = ( micros() - prevmillis ); // Time difference between revolution in microsecond
         prevmillis = micros(); // store time for next revolution calculation
         rpm = (60000000/duration); // rpm = (1/ time millis)*1000*1000*60;
       }
   }
  prevstate = currentstate; // store this scan (prev scan) data for next scan
}

void blinkLEDs(){
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  delay(1000);
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  delay(1000);
}
// End
