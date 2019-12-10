/* 
 *  NeoTrellis Grid
 *  
 *  Many thanks to scanner_darkly, Szymon Kaliski, John Park, todbot, Juanma and others
 *
*/

#include "MonomeSerialDevice.h"
#include <Adafruit_NeoTrellis.h>
#include <Arduino.h>

// IF USING ADAFRUIT M0 or M4 BOARD
#define M0 0
//#include <Arduino.h>
//#include <Adafruit_TinyUSB.h>
//#include <elapsedMillis.h>


#define Y_DIM 8 //number of rows of keys across
#define X_DIM 4 //number of columns of keys down
#define INT_PIN 9
#define BRIGHTNESS 50 // is this used?

#define NUMTRELLIS 2
#define NUM_KEYS (NUMTRELLIS * 16)


// set your monome device name here
String deviceID = "neo-monome";

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";


// Monome class setup
#define MONOMEDEVICECOUNT 1
MonomeSerialDevice mdp;
elapsedMillis monomeRefresh;

// R,G,B Values for grid color
uint8_t GridColor[] = { 255,185,0 }; // amber? {255,200,0}

// NeoTrellis setup
Adafruit_NeoTrellis trellis_array[Y_DIM / 4][X_DIM / 4] = {
    { Adafruit_NeoTrellis(0x2E) },
    { Adafruit_NeoTrellis(0x2F) }
/*   , Adafruit_NeoTrellis(0x2E) ,Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x31)},
    { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x33),Adafruit_NeoTrellis(0x34), Adafruit_NeoTrellis(0x35) }*/
};
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, Y_DIM / 4, X_DIM / 4);


// is this used?
uint32_t prevReadTime = 0;
uint32_t prevWriteTime = 0;
uint8_t currentWriteX = 0;


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
TrellisCallback blink(keyEvent evt){
  uint8_t x;
  uint8_t y;
  x  = evt.bit.NUM % 16; // Y_DIM;
  y = evt.bit.NUM / 16; // X_DIM;
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
  sendLeds();
  trellis.show();
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
  mdp.setupAsGrid(Y_DIM, X_DIM);

//  delay(200);
//  mdp.getDeviceInfo();


	if(!trellis.begin()){
		Serial.println("failed to begin trellis");
		while(1);
	}
	//trellis.setBrightness(BRIGHTNESS);

	uint8_t x, y;

// rainbow startup 
  for(int i=0; i<Y_DIM*X_DIM; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, X_DIM*Y_DIM, 0, 255))); //addressed with keynum
      trellis.show();
      delay(10);
  }
	for (x = 0; x < X_DIM; x++) {
		for (y = 0; y < Y_DIM; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, blink);
      trellis.setPixelColor(x, y, 0x000000); //addressed with x,y
      trellis.show(); //show all LEDs
      delay(50);
		}
	}
  //Serial.println("--NeoTrellis Monome--");

}

void sendLeds(){
  uint8_t r, g, b, l;
  uint32_t hexval;
  r = GridColor[0];
  g = GridColor[1];
  b = GridColor[2];
  
  for(int i=0; i<Y_DIM*X_DIM; i++){
    l = mdp.leds[i];
    hexval = (((r * l) >> 4) << 16) + (((g * l) >> 4) << 8) + ((b * l) >> 4);
    trellis.setPixelColor(i, hexval);
  }
}


// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {
	//unsigned long now = millis();

    mdp.poll(); // process incoming serial from Monomes
    
    trellis.read();

    if (monomeRefresh > 50) {
        for (int i = 0; i < MONOMEDEVICECOUNT; i++) {
          //mdp.refresh(); 
          sendLeds();
          trellis.show();

        }
        monomeRefresh = 0;
    }

	delay(20); // What's this about?
}
