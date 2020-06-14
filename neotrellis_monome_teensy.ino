/* 
 *  NeoTrellis Grid
 *  
 *  Many thanks to scanner_darkly, Szymon Kaliski, John Park, todbot, Juanma and others
 *
*/
#include "MonomeSerialDevice.h"
#include <Adafruit_NeoTrellis.h>
#include "debug.h"

// IF USING ADAFRUIT M0 or M4 BOARD
#define M0 0
//#include <Arduino.h>
//#include <Adafruit_TinyUSB.h>
//#include <elapsedMillis.h>


#define NUM_ROWS 8 // DIM_Y number of rows of keys down
#define NUM_COLS 16 // DIM_X number of columns of keys across

#define INT_PIN 9
#define LED_PIN 13 // teensy LED 

// This assumes you are using a USB breakout board to route power to the board 
// If you are plugging directly into the teensy, you will need to adjust this brightness to a much lower value
#define BRIGHTNESS 127 // overall grid brightness - use gamma table below to adjust levels

#define R 255
#define G 255
#define B 255
//  amber? {255,191,0}
//  warmer white? {255,255,200}

// set your monome device name here
String deviceID = "neo-monome";

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";

bool isInited = false;
elapsedMillis monomeRefresh;


// Monome class setup
MonomeSerialDevice mdp;

int prevLedBuffer[mdp.MAXLEDCOUNT]; 

// NeoTrellis setup
/*
// 8x8 setup
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F) },
  { Adafruit_NeoTrellis(0x36), Adafruit_NeoTrellis(0x3E) }
};
*/
// 16x8 
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)}, // top row
  { Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x3E), Adafruit_NeoTrellis(0x36) } // bottom row
};

Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, NUM_ROWS / 4, NUM_COLS / 4);

// gamma table for 16 levels of brightness
const uint8_t gammaTable[16] = { 2,  3,  4,  6,  11, 18, 25, 32, 41, 59, 70, 80, 92, 103, 115, 128}; 



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
    //on
    //Serial.println(" pressed ");
    mdp.sendGridKey(x, y, 1);
  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    //off 
    //Serial.println(" released ");
    mdp.sendGridKey(x, y, 0);
  }
  //sendLeds();
  //trellis.show();
  return 0;
  
}

// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************

void setup(){
/*
// for Adafruit M0 or M4
  pad_with_nulls(mfgstr, 32);
  pad_with_nulls(prodstr, 32);
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);
*/
	Serial.begin(115200);
  
  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);

//  delay(200);

  trellis.begin();
  
/*	if(!trellis.begin()){
		Serial.println("failed to begin trellis");
		while(1);
	}
*/
  // set overall brightness for all pixels
  uint8_t x, y;
  for (x = 0; x < NUM_COLS / 4; x++) {
    for (y = 0; y < NUM_ROWS / 4; y++) {
      trellis_array[y][x].pixels.setBrightness(BRIGHTNESS);
    }
  }

// rainbow startup 
/*
  for(int i=0; i<NUM_ROWS*NUM_COLS; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, NUM_ROWS*NUM_COLS, 0, 255))); //addressed with keynum
      trellis.show();
      delay(2);
  }
*/ 
  // key callback
	for (x = 0; x < NUM_COLS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, keyCallback);
		}
	}
  delay(300);
  mdp.setAllLEDs(0);
  sendLeds();
  monomeRefresh = 0;
  isInited = true;
  // blink one led to show it's started up  
  trellis.setPixelColor(0, 0xFFFFFF);
  trellis.show();
  delay(100);
  trellis.setPixelColor(0, 0x000000);
  trellis.show();
}

// ***************************************************************************
// **                                SEND LEDS                              **
// ***************************************************************************

void sendLeds(){
  uint8_t value, prevValue = 0;
  uint32_t hexColor;
  bool isDirty = false;

  for(int i=0; i< NUM_ROWS * NUM_COLS; i++){ 
    value = mdp.leds[i];
    prevValue = prevLedBuffer[i];
    uint8_t gvalue = gammaTable[value];

    if (value != prevValue) {
      //hexColor = (((R * value) >> 4) << 16) + (((G * value) >> 4) << 8) + ((B * value) >> 4);
      hexColor =  (((gvalue*R)/256) << 16) + (((gvalue*G)/256) << 8) + (((gvalue*B)/256) << 0);
      trellis.setPixelColor(i, hexColor);

      prevLedBuffer[i] = value;
      isDirty = true;
    }
  }
 
  if (isDirty) {
    trellis.show();
  }
}



// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {

    mdp.poll(); // process incoming serial from Monomes
 
    // refresh every 16ms or so
    if (isInited && monomeRefresh > 16) {
        trellis.read();
        sendLeds();
        monomeRefresh = 0;
    }

}
