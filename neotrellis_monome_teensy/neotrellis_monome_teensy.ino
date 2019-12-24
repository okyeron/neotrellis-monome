/* 
 *  NeoTrellis Grid
 *  
 *  Many thanks to scanner_darkly, Szymon Kaliski, John Park, todbot, Juanma and others
 *
*/
#include <Adafruit_NeoTrellis.h>


// IF USING ADAFRUIT M0 or M4 BOARD
#define M0 0
//#include <Arduino.h>
//#include <Adafruit_TinyUSB.h>
//#include <elapsedMillis.h>


#define NUM_ROWS 8 // DIM_Y number of rows of keys down
#define NUM_COLS 16 // DIM_X number of columns of keys across
#define NUM_LEDS NUM_ROWS*NUM_COLS

#define INT_PIN 9
#define LED_PIN 13 // teensy LED used to show boot info

#define BRIGHTNESS 35 // overall grid brightnes - 35 seems to be OK for all leds at full brightness
#define R 255
#define G 255
#define B 200
//  amber? {255,191,0}
//  warmer white? {255,255,200}

// R,G,B Values for grid color
uint8_t GridColor[] = { R,G,B }; 

// set your monome device name here
String deviceID = "neo-monome";

bool active;
bool isMonome;
bool isGrid;
uint8_t rows;
uint8_t columns;
uint8_t encoders;
uint8_t gridX;
uint8_t gridY;
static const int variMonoThresh = 0;
static const int MAXLEDCOUNT = 128;
uint8_t leds[MAXLEDCOUNT];
bool arcDirty = false;
bool gridDirty = false;

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";

bool isInited = false;
elapsedMillis monomeRefresh;

int prevLedBuffer[MAXLEDCOUNT];  // ????


// NeoTrellis setup
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)}, // top row
  { Adafruit_NeoTrellis(0x35), Adafruit_NeoTrellis(0x39), Adafruit_NeoTrellis(0x3F), Adafruit_NeoTrellis(0x37) } // bottom row
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
    //on
    //Serial.println(" pressed ");
    sendGridKey(x, y, 1);
  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    //off 
    //Serial.println(" released ");
    sendGridKey(x, y, 0);
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
  
  isMonome = true;
  setupAsGrid(NUM_ROWS, NUM_COLS);

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
  for(int i=0; i<NUM_ROWS*NUM_COLS; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, NUM_ROWS*NUM_COLS, 0, 255))); //addressed with keynum
      trellis.show();
      delay(2);
  }
  
  // key callback
	for (x = 0; x < NUM_COLS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, keyCallback);
		}
	}
  delay(500);
  setAllLEDs(0);
  sendLeds();
  monomeRefresh = 0;
  isInited = true;
}

// ***************************************************************************
// **                                MONOME SERIAL                          **
// ***************************************************************************

void initialize() {
    active = false;
    isMonome = false;
    isGrid = true;
    rows = 0;
    columns = 0;
    encoders = 0;
    clearAllLeds();
    arcDirty = false;
    gridDirty = false;
}

void setupAsGrid(uint8_t _rows, uint8_t _columns) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = true;
    rows = _rows;
    columns = _columns;
    gridDirty = true;
}
void setupAsArc(uint8_t _encoders) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = false;
    encoders = _encoders;
    arcDirty = true;
}
void getDeviceInfo() {
    Serial.write(uint8_t(0));
 }

void poll() {
    //while (isMonome && Serial.available()) { processSerial(); };
    if (Serial.available()) {
      processSerial();
    }
}

void setAllLEDs(int value) {
  for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = value;
}

void setGridLed(uint8_t x, uint8_t y, uint8_t level) {
    if (x >= columns || y >= rows) {
      return;
    }
    int index;
    if (columns > 8){
      index = x + (y << 4);
    }else{
      index = x + (y << 3);
    }
    if (index < MAXLEDCOUNT) leds[index] = level;
}
        
void clearGridLed(uint8_t x, uint8_t y) {
    setGridLed(x, y, 0);
}

void setArcLed(uint8_t enc, uint8_t led, uint8_t level) {
    int index = led + (enc << 6);
    if (index < MAXLEDCOUNT) leds[index] = level;
}
        
void clearArcLed(uint8_t enc, uint8_t led) {
    setArcLed(enc, led, 0);
}

void clearAllLeds() {
    for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = 0;
}

void clearArcRing(uint8_t ring) {
    for (int i = ring << 6, upper = i + 64; i < upper; i++) leds[i] = 0;
}

void processSerial() {    
    uint8_t identifierSent;  // command byte sent from controller to matrix

    uint8_t index, readX, readY, readN, readA;
    uint8_t dummy, gridNumber, i2cAddress;  // for reading in data not used by the matrix
    uint8_t n, x, y, z, i;
    uint8_t intensity = 15;
    uint8_t gridKeyX;
    uint8_t gridKeyY;
    int8_t delta;
    uint8_t gridX    = columns;          // Will be either 8 or 16
    uint8_t gridY    = rows;           // standard for 128
    uint8_t numQuads = columns/rows;
    
    // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
    // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
    // b = command (ie. query, enable, led, key, frame)

    identifierSent = Serial.read();  

    switch (identifierSent) {
        case 0x00:  // device information
        	// [null, "led-grid", "key-grid", "digital-out", "digital-in", "encoder", "analog-in", "analog-out", "tilt", "led-ring"]
            Serial.write((uint8_t)0x00); // action: response, 0x00 = system
            Serial.write((uint8_t)0x01); // section id, 1 = led-grid, 2 = key-grid, 5 = encoder/arc	## NEED devSect variable
            Serial.write((uint8_t)numQuads);   // one Quad is 64 buttons
            break;

        case 0x01:  // system / ID
            Serial.write((uint8_t)0x01);        // action: response, 0x01
            for (i = 0; i < 32; i++) {          // has to be 32
                if (i < deviceID.length()) {
                  Serial.write(deviceID[i]);
                } else {
                  Serial.write(0x00);
                }
            }
            break;

        case 0x02:  // system / write ID
            //Serial.println("0x02");
            for (int i = 0; i < 32; i++) {  // has to be 32
                deviceID[i] = Serial.read();
            }
            break;

        case 0x03:  // system / report grid offset
            //Serial.println("0x03");
            Serial.write((uint8_t)0x02);     // system / request grid offset - bytes: 1 - [0x03]
            Serial.write((uint8_t)0x01);
            Serial.write((uint8_t)0);     // x offset - could be 0 or 8  ### NEEDS grid size variable
            Serial.write((uint8_t)0);     // y offset
            break;

        case 0x04:  // system / report ADDR
            //Serial.println("0x04");
            gridNumber = Serial.read();        // grid number
            readX = Serial.read();          // x offset
            readY = Serial.read();          // y offset
            break;

        case 0x05:
            //Serial.println("0x05");
            Serial.write((uint8_t)0x03);             // system / request grid size
            Serial.write((uint8_t)gridX);                // gridX
            Serial.write((uint8_t)gridY);                // gridY
            break;

        case 0x06:
            readX = Serial.read();          // system / set grid size - ignored
            readY = Serial.read();
            break;

        case 0x07:
            break;                              // I2C get addr (scan) - ignored

        case 0x08:
            i2cAddress = Serial.read();     // I2C set addr - ignored
            dummy = Serial.read();
            break;


        case 0x0F:  // system / report firmware version
            // Serial.println("0x0F");
            for (int i = 0; i < 8; i++) {  // 8 character string
                //Serial.print(Serial.read());
            }
            break;


      // 0x10-0x1F are for an LED Grid Control.  All bytes incoming, no responses back
  
        case 0x10:            // /prefix/led/set x y [0/1]  / led off
          readX = Serial.read();
          readY = Serial.read();
          setGridLed(readX, readY, 0);
          break;

        case 0x11:            // /prefix/led/set x y [0/1]   / led on
          readX = Serial.read();
          readY = Serial.read();
          setGridLed(readX, readY, 15);   // need global brightness variable?
          break;

        case 0x12:            //  /prefix/led/all [0/1]  / all off
          clearAllLeds();
          break;

        case 0x13:                      //  /prefix/led/all [0/1] / all on
          setAllLEDs(15);
          break;

        case 0x14:                  // /prefix/led/map x y d[8]  / map (frame)
          readX = Serial.read();
          readY = Serial.read();

          for (y = 0; y < 8; y++) {           // each i will be a row
            intensity = Serial.read();            // read one byte of 8 bits on/off
    
            for (x = 0; x < 8; x++) {          // for 8 LEDs on a row
              if ((intensity >> x) & 0x01) {      // set LED if the intensity bit is set
                setGridLed(readX + x, y, intensity); 
              }
              else {
                setGridLed(readX + x, y, 0); 
              }
            }
          }
          break;

        case 0x15:                                //  /prefix/led/row x y d
          readX = Serial.read();                      // led-grid / set row
          readY = Serial.read();                      // 
          intensity = Serial.read();                  // read one byte of 8 bits on/off

          for (x = 0; x < 8; x++) {               // for the next 8 lights in row
            if ((intensity >> x) & 0x01) {        // if intensity bit set, light led
              setGridLed(readX + x, readY, intensity);
            } else {
              setGridLed(readX + x, readY, 0);
            }
          }

          break;

        case 0x16:                                //  /prefix/led/col x y d
          readX = Serial.read();                      // led-grid / column set
          readY = Serial.read();
          intensity = Serial.read();                  // read one byte of 8 bits on/off

          for (y = 0; y < 8; y++) {           // for the next 8 lights in column
            if ((intensity >> y) & 0x01) {        // if intensity bit set, light led
              setGridLed(readX, readY + y, intensity);
            } else {
              setGridLed(readX, readY + y, 0);
            }
          }

          break;

        case 0x17:                                     //  /prefix/led/intensity i
          intensity = Serial.read();                      // set brightness for entire grid
          // this is probably not right
          setAllLEDs(intensity);

          break;

        case 0x18:                                //  /prefix/led/level/set x y i
          readX = Serial.read();                      // led-grid / set LED intensity
          readY = Serial.read();                      // read the x and y coordinates
          intensity = Serial.read();                  // read the intensity
          setGridLed(readX, readY, intensity);              
          break;

        case 0x19:                               //  /prefix/led/level/all s
          intensity = Serial.read();                 // set all leds
          setAllLEDs(intensity);              
          break;

        case 0x1A:                               //   /prefix/led/level/map x y d[64]
                                                 // set 8x8 block
         readX = Serial.read();                      // x offset
         readY = Serial.read();                      // y offset
  
          z = 0;
          for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
              if (z % 2 == 0) {                    
                intensity = Serial.read();
                 // even bytes, use upper nybble
                 setGridLed(readX + x, readY + y, (intensity >> 4) & 0x0F);
              } else {                        
                // odd bytes, use lower nybble
                setGridLed(readX + x, readY + y, intensity & 0x0F);
              }
              z++;
            }
          }
            
         /*
          } else {
            for (int q = 0; q<32; q++){
              Serial.read();
            }
          }*/
          break;

        case 0x1B:                                // /prefix/led/level/row x y d[8]
          readX = Serial.read();                      // set 8x1 block of led levels, with offset
          readY = Serial.read();                      // y = y offset

          for (x = 0; x < 8; x++) {
              if (x % 2 == 0) {                    
                intensity = Serial.read();          
                // even bytes, use upper nybble
                setGridLed(readX + x, readY, intensity);
              } else {                              
                // odd bytes, use lower nybble
                setGridLed(readX + x, readY, intensity);
              }
          }
          break;

        case 0x1C:                                // /prefix/led/level/col x y d[8]
          readX = Serial.read();                      // set 1x8 block of led levels, with offset
          readY = Serial.read();                      // x = x offset

          for (y = 0; y < 8; y++) {
              if (y % 2 == 0) {                    
                intensity = Serial.read();
                // even bytes, use upper nybble
                setGridLed(readX, readY + y, intensity);
              } else {                              
                // odd bytes, use lower nybble
                setGridLed(readX, readY + y, intensity);
              }
          }
          break;

        // 0x90 variable 64 LED ring 
        case 0x90:
          //pattern:  /prefix/ring/set n x a
          //desc:   set led x of ring n to value a
          //args:   n = ring number
          //      x = led number
          //      a = value (0-15)
          //serial:   [0x90, n, x, a]
          readN = Serial.read();
          readX = Serial.read();
          readA = Serial.read();
          //led_array[readN][readX] = readA;
          setArcLed(readN, readX, readA);         
          break;
     
        case 0x91:
          //pattern:  /prefix/ring/all n a
          //desc:   set all leds of ring n to a
          //args:   n = ring number
          //      a = value
          //serial:   [0x91, n, a]
          readN = Serial.read();
          readA = Serial.read();
          for (int q=0; q<64; q++){
            setArcLed(readN, q, readA);
            //led_array[readN][q]=readA;
          }
          break;
      
        case 0x92:
          //pattern:  /prefix/ring/map n d[32]
          //desc:   set leds of ring n to array d
          //args:   n = ring number
          //      d[32] = 64 states, 4 bit values, in 32 consecutive bytes
          //      d[0] (0:3) value 0
          //      d[0] (4:7) value 1
          //      d[1] (0:3) value 2
          //      ....
          //      d[31] (0:3) value 62
          //      d[31] (4:7) value 63
          //serial:   [0x92, n d[32]]
          readN = Serial.read();
          for (y = 0; y < 64; y++) {
              if (y % 2 == 0) {                    
                intensity = Serial.read();
                if ( (intensity >> 4 & 0x0F) > 0) {  // even bytes, use upper nybble
                  //led_array[readN][y] = (intensity >> 4 & 0x0F);
                  setArcLed(readN, y, (intensity >> 4 & 0x0F)); 
                }
                else {
                  //led_array[readN][y]=0;
                  setArcLed(readN, y, 0);   
                }
              } else {                              
                if ((intensity & 0x0F) > 0 ) {      // odd bytes, use lower nybble
                  //led_array[readN][y] = (intensity & 0x0F);
                  setArcLed(readN, y, (intensity & 0x0F));
                }
                else {
                  //led_array[readN][y]=0;
                  setArcLed(readN, y, 0);
                }
              }
          }
          break;

        case 0x93:
          //pattern:  /prefix/ring/range n x1 x2 a
          //desc:   set leds inclusive from x1 and x2 of ring n to a
          //args:   n = ring number
          //      x1 = starting position
          //      x2 = ending position
          //      a = value
          //serial:   [0x93, n, x1, x2, a]
          readN = Serial.read();
          readX = Serial.read();  // x1
          readY = Serial.read();  // x2
          readA = Serial.read();
          //memset(led_array[readN],0,sizeof(led_array[readN]));
      
          if (readX < readY){
            for (y = readX; y < readY; y++) {
              //led_array[readN][y] = readA;
              setArcLed(readN, y, readA);
            }
          }else{
            // wrapping?
            for (y = readX; y < 64; y++) {
              //led_array[readN][y] = readA;
              setArcLed(readN, y, readA);
            }
            for (x = 0; x < readY; x++) {
              //led_array[readN][x] = readA;
              setArcLed(readN, y, readA);
            }
          }
          //note:   set range x1-x2 (inclusive) to a. wrapping supported, ie. set range 60,4 would set values 60,61,62,63,0,1,2,3,4. 
          // always positive direction sweep. ie. 4,10 = 4,5,6,7,8,9,10 whereas 10,4 = 10,11,12,13...63,0,1,2,3,4 
         break;

        default: 
          break;
    }
}

void sendArcDelta(uint8_t index, int8_t delta) {
    Serial.write((uint8_t)0x50);
    Serial.write((uint8_t)index);
    Serial.write((int8_t)delta);
}

void sendArcKey(uint8_t index, uint8_t pressed) {
    uint8_t buf[2];
    if (pressed == 1){
      buf[0] = 0x52;
    }else{
      buf[0] = 0x51;
    }
    buf[1] = index;
    Serial.write(buf, 2);
}

void sendGridKey(uint8_t x, uint8_t y, uint8_t pressed) {    
    uint8_t buf[2];
    if (pressed == 1){
      buf[0] = 0x21;
    }else{
      buf[0] = 0x20;
    }
    Serial.write((uint8_t)buf[0]);
    Serial.write((uint8_t)x);
    Serial.write((uint8_t)y);
}


// ***************************************************************************

void sendLeds(){
  uint8_t value;
  uint8_t r, g, b;
  uint32_t hexColor;
  r = GridColor[0];
  g = GridColor[1];
  b = GridColor[2];
  bool isDirty = false;

  for(int i=0; i< NUM_ROWS * NUM_COLS; i++){
    value = leds[i];
    hexColor = (((r * value) >> 4) << 16) + (((g * value) >> 4) << 8) + ((b * value) >> 4);
    trellis.setPixelColor(i, hexColor);
  }
  trellis.show();

}


// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {

    poll(); // process incoming serial from Monomes
 
    // refresh every 16ms or so
    if (isInited && monomeRefresh > 16) {
        monomeRefresh = 0;
        trellis.read();
        sendLeds();
    }

}
