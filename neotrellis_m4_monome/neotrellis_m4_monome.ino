// SET TOOLS USB STACK TO TinyUSB

#include "NeoTrellisM4.h"
#include "MonomeSerialDevice.h"
#include <Adafruit_TinyUSB.h>
#include <elapsedMillis.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>


#define NUM_ROWS 4 // DIM_Y number of rows of keys down
#define NUM_COLS 8 // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS*NUM_COLS

#define BRIGHTNESS 255 // overall grid brightness - use gamma table below to adjust levels

#define R 255
#define G 255
#define B 255

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

// accelerometer 
Adafruit_ADXL343 accel = Adafruit_ADXL343(123, &Wire1);


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


// floating point map
float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    if (clamp) {
      if (outputMax < outputMin) {
        if (outVal < outputMax)  outVal = outputMax;
        else if (outVal > outputMin)  outVal = outputMin;
      } else {
        if (outVal > outputMax) outVal = outputMax;
        else if (outVal < outputMin)  outVal = outputMin;
      }
    }
    return outVal;
}

// ***************************************************************************
// **                          FUNCTIONS FOR TRELLIS                        **   
// ***************************************************************************


void tiltevent(int16_t x,int16_t y, int16_t z) {

 /* 
  int xtilt = 0;
  int ytilt = 0;
  int ztilt = 0;
  int last_xtilt = 0;
  int last_ytilt = 0;
  int last_ztilt = 0;

  if (abs(y) < 2.0) {  // 2.0 m/s^2
    // don't make any bend unless they've really started moving it
    ytilt = 0; // no bend
  } else {
    if (ytilt > 0) {
      ytilt = ofMap(y, 2.0, 10.0, 0, 255, true);  // 2 ~ 10 m/s^2 is downward bend
    } else {
      ytilt = ofMap(y, -2.0, -10.0, -255, 0, true);  // -2 ~ -10 m/s^2 is upward bend
    }
  }
  if (ytilt != last_ytilt) {
    // do tilt
    last_ytilt = ytilt;
  }

  if (abs(x) < 2.0) {  // 2.0 m/s^2
    // don't make any bend unless they've really started moving it
    xtilt = 0;
  } else {
    if (xtilt > 0) {
      xtilt = ofMap(x, 2.0, 10.0, 255, 0, true);  // 2 ~ 10 m/s^2 is upward bend
    } else {
      xtilt = ofMap(x, -2.0, -10.0, -255, 0, true);  // -2 ~ -10 m/s^2 is downward bend
    }
  }
  if (xtilt != last_xtilt) {
    // do tilt
    last_xtilt = xtilt;
  }
*/
  int8_t xh = (x>>8) & 0xff;  
  int8_t xl = x & 0xff;  
  int8_t yh = (y>>8) & 0xff;  
  int8_t yl = y & 0xff;  
  int8_t zh = (z>>8) & 0xff;  
  int8_t zl = z & 0xff;  

  mdp.sendTiltEvent(0x01, xh, xl,  yh, yl, zh, zl);

}

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
  accel.begin();
  
  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);

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
    trellis.tick();

    mdp.poll(); // process incoming serial from Monomes

    sensors_event_t event;
    accel.getEvent(&event);
    tiltevent(event.acceleration.x,event.acceleration.y,event.acceleration.z);

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
