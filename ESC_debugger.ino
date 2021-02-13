#include <avr/io.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);




float batt3cellVolt=0;

void setup(){
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  sendOLEDWelcomeText();
  
  setPinModes();
}
void loop(){
  //Serial.println("It's working");
  batt3cellVolt=(analogRead(A0)*5.0)/1024.0;
  //Serial.println((analogRead(A0)*5)>>10);
  blinkLEDs();

  sendToOLED(batt3cellVolt*((1.5+1)/1));

  OLEDMenu();
}









void sendToOLED(float v) {
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.setTextColor(WHITE); 
  display.setTextSize(2); 
 
  display.print(v);display.print(F("v "));//battery capacity
  display.display();
}

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
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("0: Debug ESC"));
  display.println(F("1: Config ESC"));
  display.println(F("2: LED Delay"));
  display.display();
  delay(1000);
}

void blinkLEDs(){
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  delay(1000);
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  delay(1000);
}
void setPinModes(){
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
}
