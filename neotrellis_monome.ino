/* 
 *  NeoTrellis Grid
 *  
*/

#include "Adafruit_NeoTrellis.h"
#include "MonomeSerialDevice.h"

// IF USING ADAFRUIT M0 or M4 BOARD
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>


#define Y_DIM 4 //number of rows of key
#define X_DIM 8 //number of columns of keys
#define INT_PIN 9
#define BRIGHTNESS 50

#define NUMTRELLIS 1
#define numKeys (NUMTRELLIS * 16)


// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";

// Monome setup
#define MONOMEDEVICECOUNT 1

// set your monome device name here
String deviceID = "neotrellis-monome";


MonomeSerial monomeDevices;
//monomeDevices->isMonome = true;
//monomeDevices->setupAsGrid(8, 16);
//elapsedMillis monomeRefresh;


Adafruit_NeoTrellis trellis_array[Y_DIM / 4][X_DIM / 4] = {
    { Adafruit_NeoTrellis(0x2F) }
/*    , Adafruit_NeoTrellis(0x2F),Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x31)},
    { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x33),Adafruit_NeoTrellis(0x34), Adafruit_NeoTrellis(0x35) }*/
};
     
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, Y_DIM / 4, X_DIM / 4);


boolean *lit_keys;

uint32_t ledBuffer[X_DIM][Y_DIM];
uint32_t prevReadTime = 0;
uint32_t prevWriteTime = 0;
uint8_t currentWriteX = 0;

// Pad a string of length 'len' with nulls
void pad_with_nulls(char* s, int len) {
  int l = strlen(s);
  for( int i=l;i<len; i++) {
    s[i] = '\0';
  }
}

// ***************************************************************************
// **                          FUNCTIONS FOR TRELLIS                        **   
// ***************************************************************************




// ***************************************************************************
// **                                HELPERS                                **
// ***************************************************************************


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

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING){
    // press stuff
    Serial.println(" pressed ");
    trellis.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, X_DIM*Y_DIM, 0, 255))); //on rising

    //monomeDevices.sendGridKey(x, y, 1);

  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    // release stuff
    Serial.println(" released ");
    trellis.setPixelColor(evt.bit.NUM, 0); //off falling
 
    //monomeDevices.sendGridKey(x, y, 0);
    
  trellis.show();
  return 0;
  }
}

// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************

void setup(){
	Serial.begin(115200);

  Serial.println("--NeoTrellis Monome--");

  pad_with_nulls(mfgstr, 32);
  pad_with_nulls(prodstr, 32);
  
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);


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
      delay(50);
  }
	for (x = 0; x < X_DIM; x++) {
		for (y = 0; y < Y_DIM; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, blink);
      trellis.setPixelColor(x, y, 0x000000); //addressed with x,y
      trellis.show(); //show all LEDs
		}
	}

}

// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {
	unsigned long now = millis();

   if (Serial.available() > 0) {
      do {
//        processSerial();
      } while (Serial.available() > 16);
    }

     trellis.read();
 
  
	//delay(10);
}
