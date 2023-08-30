// SET TOOLS USB STACK TO TinyUSB

#include "NeoTrellisM4.h"
#include "MonomeSerialDevice.h"
#include <Adafruit_TinyUSB.h>
#include <elapsedMillis.h>

#define NUM_ROWS 4 // DIM_Y number of rows of keys down
#define NUM_COLS 8 // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS*NUM_COLS

#define BRIGHTNESS 255 // overall grid brightness - use gamma table below to adjust levels

// RGB values - vary for colors other than white
#define DEFAULT_R 255
#define DEFAULT_G 255
#define DEFAULT_B 255

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4676123";

// set your monome device name here
String deviceID = "neo-monome";
String serialNum = "m4676123";

elapsedMillis monomeRefresh;
bool isInited = false;

int prevLedBuffer[32];

// Monome class setup
MonomeSerialDevice mdp;


// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
NeoTrellisM4 trellis = NeoTrellisM4();

// gamma table for 16 levels of brightness
const uint8_t gammaTable[16] = { 0,  2,  3,  6,  11, 18, 25, 32, 41, 59, 70, 80, 92, 103, 115, 128};


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


// ***************************************************************************
// **                          FUNCTIONS FOR TRELLIS                        **
// ***************************************************************************


// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************


void setup() {
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
  mdp.R = DEFAULT_R;
  mdp.G = DEFAULT_G;
  mdp.B = DEFAULT_B;

  trellis.begin();

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

    if (value != prevValue || mdp.colorDirty) {
      //hexColor = (((mdp.R * value) >> 4) << 16) + (((mdp.G * value) >> 4) << 8) + ((mdp.B * value) >> 4);
      hexColor =  (((gvalue*mdp.R)/256) << 16) + (((gvalue*mdp.G)/256) << 8) + (((gvalue*mdp.B)/256) << 0);
      trellis.setPixelColor(i, hexColor);

      prevLedBuffer[i] = value;
      isDirty = true;
    }
  }
  if (isDirty) {
    trellis.show();
    mdp.colorDirty = false;
  }

}



// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {
    trellis.tick();

    mdp.poll(); // process incoming serial from Monomes

    // refresh every 16ms or so
    if (isInited && monomeRefresh > 16) {
      keypadEvent e = trellis.read();
      uint8_t x  = e.bit.KEY % NUM_COLS;
      uint8_t y = e.bit.KEY / NUM_COLS;

      //Serial.print((int)e.bit.KEY);

      if (e.bit.EVENT == KEY_JUST_PRESSED) {
        mdp.sendGridKey(x, y, 1);
        //Serial.println(" pressed");
        //trellis.setPixelColor(e.bit.KEY, 0x111111);
      }
      else if (e.bit.EVENT == KEY_JUST_RELEASED) {
        mdp.sendGridKey(x, y, 0);
        //Serial.println(" released");
        //trellis.setPixelColor(e.bit.KEY, 0x0);
      }
        sendLeds();
        monomeRefresh = 0;
    }

}
