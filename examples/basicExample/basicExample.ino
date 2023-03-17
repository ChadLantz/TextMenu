#include "WiFi.h"
#include <Wire.h>
#include "SSD1306Wire.h"
#include "TextMenu.h"
#include "image.h"

//Make the display
int screenW = 128;
int screenH = 64;
int lineHeight = 11;
int targetFPS = 30;
int targetFrameTime = 1000/targetFPS;//target frame time rounded down to the nearest ms
int lastFrameTime = 0;
SSD1306Wire display(0x3c, SDA, SCL);

//Make our buttons and menu
//Be sure to set the pin numbers to the GPIO your buttons are connected to
ezButton Up(9);
ezButton Down(8);
ezButton Enter(7);
ezButton Back(6);
TextMenu menu("Main", Up, Down, Enter, Back);


//A somewhat simple function. Draws a picture and returns the false if the back button
//was pressed indicating this function should no longer be drawn. Returns true if this function
//should continue to be drawn 
bool doSomething(){
  display.clear(); //Every function that uses the display should start with this

  //Handle button presses
  if(Back.isPressed()) return false;

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawXbm(0, 0, doSomething_width, doSomething_height, doSomethingImage);
  display.drawString(30, 15, "c'mon,");
  display.drawString(30, 25, "do something...");

  display.display(); //Every function that uses the display should end with this
  return true;
}



//Define some parameters for the next function
int scanInterval = 10000; 
int lastScan = 0;
int startIndex = 0;
int nLines = screenH/lineHeight;

//A more complex function. Performs a WiFi scan and displays the SSID and signal strength
bool wifiScan(){
  display.clear();

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanComplete();
  if( n <= 0 || (millis() - lastScan) > scanInterval ){
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(screenW/2,screenH/2, "Scanning");
    display.display();

    WiFi.scanDelete();
    n = WiFi.scanNetworks();
    lastScan = millis();
  }

  //Handle button presses
  if(Back.isPressed()){ 
    return false;
  }else if(Up.isPressed() && startIndex > 0){//Don't scroll past the top of the list
    startIndex--;
  }else if(Down.isPressed() && startIndex < n-2-nLines){//Don't scroll to the point where we draw empty lines
    startIndex++;
  }

  if (n == 0) {
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(screenW/2, screenH/2, "no networks found");
  } else {
    for (int i = startIndex; i < min(nLines + startIndex,n-startIndex); ++i) {

      int lineY = (i - startIndex)*lineHeight;
      // Print SSID and RSSI for each network found
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(10, lineY, WiFi.SSID(i).substring(0,15));
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(screenW, lineY, String(WiFi.RSSI(i)) + " dB");
    }
  }
  display.display();
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, screenH/2,"In setup");
  display.display();

  Serial.println("In setup");

  menu.setLineHeight(11);
  TextMenu *subMenu = menu.addSubMenu("Option 1");
  TextMenu *subSubMenu = subMenu->addSubMenu("Sub Option 1");

  //Add some filler to demonstrate scrolling
  subMenu->addEntry("Does nothing 1", nullptr);
  subMenu->addEntry("Does nothing 2", nullptr);
  subMenu->addEntry("Does nothing 3", nullptr);
  subMenu->addEntry("Does nothing 4", nullptr);
  subMenu->addEntry("Does nothing 5", nullptr);
  subMenu->addEntry("Does nothing 6", nullptr);
  
  subSubMenu->addEntry("Do Something", &doSomething);
  menu.addEntry("WiFi Scan", &wifiScan);
}

void loop() {
  menu.getCurrentMenu()->draw(display);

  Serial.println("in Loop, doing things");
    //If we still have time left, delay for the remainder
  int remainingTimeBudget = lastFrameTime - millis();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
  lastFrameTime = millis();
}
