/***********************************************************
 *  DIY monome compatible grid w/ Adafruit NeoTrellis
 *  for RP2040 Pi Pico
 *  
 *  This code makes the Adafruit Neotrellis boards into a Monome compatible grid via monome's mext protocol
 *  ----> https://www.adafruit.com/product/3954
 *  
 *  Code here is for a 16x8 grid, but can be modified for 4x8, 8x8, or 16x16 (untested on larger grid arrays)
 *  
 *  Many thanks to: 
 *  scanner_darkly <https://github.com/scanner-darkly>, 
 *  TheKitty <https://github.com/TheKitty>, 
 *  Szymon Kaliski <https://github.com/szymonkaliski>, 
 *  John Park, Todbot, Juanma, Gerald Stevens, and others
 *
*/

// SET TOOLS USB STACK TO TinyUSB

// RP2040 BOARDS REQUIRE Earle Philhower's Arduino core for RP2040 devices, arduino-pico
// See https://learn.adafruit.com/rp2040-arduino-with-the-earlephilhower-core/overview

// Be sure you have these libraries installed
//    Adafruit seesaw library 
//    elapsedMillis
//    Adafruit TinyUSB Library
//    Adafruit NeoPixel

#include "MonomeSerialDevice.h"
#include <Adafruit_NeoTrellis.h>

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <elapsedMillis.h>

#include "i2c_config.h" // look here to change settings for different boards

#define NUM_ROWS 8 // DIM_Y number of rows of keys down
#define NUM_COLS 16 // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS*NUM_COLS

#define INT_PIN 9
#define LED_PIN 13 // teensy LED used to show boot info

// This assumes you are using a USB breakout board to route power to the board 
// If you are plugging directly into the controller, you will need to adjust this brightness to a much lower value
#define BRIGHTNESS 255 // overall grid brightness - use gamma table below to adjust levels

#define R 255
#define G 255
#define B 255

// gamma table for 16 levels of brightness
const uint8_t gammaTable[16] = { 0,  2,  3,  6,  11, 18, 25, 32, 41, 59, 70, 80, 92, 103, 115, 128}; 


bool isInited = false;
elapsedMillis monomeRefresh;

// set your monome device name here
String deviceID = "neo-monome";
String serialNum = "m4216126";

// DEVICE INFO FOR TinyUSB
char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4216126";

// Monome class setup
MonomeSerialDevice mdp;

int prevLedBuffer[mdp.MAXLEDCOUNT]; 


// NeoTrellis setup
/*
// 8x8 setup RP2040
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(addrRowOne[0], &MYWIRE), Adafruit_NeoTrellis(addrRowOne[1], &MYWIRE) },
  { Adafruit_NeoTrellis(addrRowTwo[0], &MYWIRE), Adafruit_NeoTrellis(addrRowTwo[1], &MYWIRE) }
};
*/
// 16x8 RP2040
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(addrRowOne[0], &MYWIRE), Adafruit_NeoTrellis(addrRowOne[1], &MYWIRE), Adafruit_NeoTrellis(addrRowOne[2], &MYWIRE), Adafruit_NeoTrellis(addrRowOne[3], &MYWIRE)}, // top row
  { Adafruit_NeoTrellis(addrRowTwo[0], &MYWIRE), Adafruit_NeoTrellis(addrRowTwo[1], &MYWIRE), Adafruit_NeoTrellis(addrRowTwo[2], &MYWIRE), Adafruit_NeoTrellis(addrRowTwo[3], &MYWIRE) } // bottom row
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
  uint8_t x  = evt.bit.NUM % NUM_COLS;
  uint8_t y = evt.bit.NUM / NUM_COLS; 

  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING){
//     Serial.println(" pressed ");
    mdp.sendGridKey(x, y, 1);
  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
//     Serial.println(" released ");
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
	uint8_t x, y;

	TinyUSBDevice.setManufacturerDescriptor(mfgstr);
	TinyUSBDevice.setProductDescriptor(prodstr);
	TinyUSBDevice.setSerialDescriptor(serialstr);
  
	Serial.begin(115200);
  
  // while( !TinyUSBDevice.mounted() ) delay(1);

	MYWIRE.setSDA(I2C_SDA);
	MYWIRE.setSCL(I2C_SCL);

Serial.println(I2C_SDA);
Serial.println(I2C_SCL);

	mdp.isMonome = true;
	mdp.deviceID = deviceID;
	mdp.setupAsGrid(NUM_ROWS, NUM_COLS);
  	monomeRefresh = 0;
  	isInited = true;

	int var = 0;
	while (var < 8) {
		mdp.poll();
		var++;
		delay(100);
	}

	if (!trellis.begin()) {
		Serial.println("trellis.begin() failed!");
		Serial.println("check your addresses.");
		Serial.println("reset to try again.");
		while(1);  // loop forever
	}

	// key callback
	for (x = 0; x < NUM_COLS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
			trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
			trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
			trellis.registerCallback(x, y, keyCallback);
		}
	}

	// set overall brightness for all pixels
	for (x = 0; x < NUM_COLS / 4; x++) {
		for (y = 0; y < NUM_ROWS / 4; y++) {
		  trellis_array[y][x].pixels.setBrightness(BRIGHTNESS);
		}
	}

	// clear grid leds
	mdp.setAllLEDs(0);
	sendLeds();
	
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
