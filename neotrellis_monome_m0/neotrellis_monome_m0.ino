/* 
 *  NeoTrellis Grid
 *  
 *  Many thanks to scanner_darkly, Szymon Kaliski, John Park, todbot, Juanma and others
 *
*/
#include "MonomeSerialDevice.h"
#include <Adafruit_NeoTrellis.h>
#include <Arduino.h>

// SET TOOLS USB STACK TO TinyUSB

// IF USING ADAFRUIT M0 or M4 BOARD
#define M0 0
#include <Adafruit_TinyUSB.h>
#include <elapsedMillis.h>


#define NUM_ROWS 8 // DIM_Y number of rows of keys down
#define NUM_COLS 8 // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS*NUM_COLS

#define INT_PIN 9
#define LED_PIN 13 // teensy LED used to show boot info

#define BRIGHTNESS 100 // overall grid brightnes - adjust lower if getting voltage droop
#define R 255
#define G 255
#define B 255

// R,G,B Values for grid color
uint8_t GridColor[] = { R,G,B }; // amber? {255,200,0}

// set your monome device name here
String deviceID = "neo-monome";
String serialNum = "m4676000";

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4676000";

elapsedMillis monomeRefresh;
bool isInited = false;

// Monome class setup
MonomeSerialDevice mdp;

// NeoTrellis setup
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F) }, // top row
  { Adafruit_NeoTrellis(0x36), Adafruit_NeoTrellis(0x3E) } // bottom row
};
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, NUM_ROWS / 4, NUM_COLS / 4);



// ***************************************************************************
// **                                HELPERS                                **
// ***************************************************************************

// Pad a string of length 'len' with nulls
void pad_with_nulls(char* s, int len) {
  int l = strlen(s);
  for( int i=l;i<len; i++) {
    s[i] = '\0';
  }
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}

// ***************************************************************************
// **                          FUNCTIONS FOR TRELLIS                        **   
// ***************************************************************************


//define a callback for key presses
TrellisCallback keyCallback(keyEvent evt){
  uint8_t x;
  uint8_t y;
  x  = evt.bit.NUM % NUM_COLS; //NUM_COLS; 
  y = evt.bit.NUM / NUM_COLS; //NUM_COLS; 
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING){
    //trellis.setPixelColor(evt.bit.NUM, 0xFFFFFF); //on rising
    //Serial.println(" pressed ");
    mdp.sendGridKey(x, y, 1);
    mdp.refreshGrid();
  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    //trellis.setPixelColor(evt.bit.NUM, 0); //off falling
    //Serial.println(" released ");
    mdp.sendGridKey(x, y, 0);
    mdp.refreshGrid();
  }
  //sendLeds();
  //trellis.show();
  return 0;
  
}

// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************

void setup(){

// for Adafruit M0 or M4
  pad_with_nulls(mfgstr, 32);
  pad_with_nulls(prodstr, 32);
  pad_with_nulls(serialstr, 32);
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);
  USBDevice.setSerialDescriptor(serialstr);

	Serial.begin(115200);
  
  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);
  mdp.setAllLEDs(0);

//  delay(200);
//  mdp.getDeviceInfo();


	if(!trellis.begin()){
		Serial.println("failed to begin trellis");
		while(1);
	}
  uint8_t x, y;
  for (x = 0; x < NUM_COLS / 4; x++) {
    for (y = 0; y < NUM_ROWS / 4; y++) {
      trellis_array[y][x].pixels.setBrightness(BRIGHTNESS);
    }
  }


// rainbow startup 
  for(int i=0; i<NUM_ROWS*NUM_COLS; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, NUM_ROWS*NUM_COLS, 0, 255))); //addressed with keynum
      trellis.show();
      delay(1);
  }
  
  // key callback
	for (x = 0; x < NUM_COLS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, keyCallback);
      trellis.setPixelColor(x, y, 0x000000); //addressed with x,y
      trellis.show(); //show all LEDs
      //delay(1);
		}
	}
  delay(500);
  isInited = true;
}


void sendLeds(){
  uint8_t value;
  uint8_t r, g, b;
  uint32_t hexColor;
  r = GridColor[0];
  g = GridColor[1];
  b = GridColor[2];

  for(int i=0; i< NUM_ROWS * NUM_COLS; i++){
    value = mdp.leds[i];
    hexColor = (((r * value) >> 4) << 16) + (((g * value) >> 4) << 8) + ((b * value) >> 4);
    trellis.setPixelColor(i, hexColor);
    
  }
  trellis.show();
  
}


// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {

    mdp.poll(); // process incoming serial from Monomes


    if (isInited && monomeRefresh > 20) {
        trellis.read();
        sendLeds();
        monomeRefresh = 0;
    }

	//delay(20); // What's this about?
}
