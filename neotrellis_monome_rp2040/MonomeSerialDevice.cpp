#include "MonomeSerialDevice.h"
#include "debug.h"

MonomeSerialDevice::MonomeSerialDevice() {}

void MonomeSerialDevice::initialize() {
    active = false;
    isMonome = false;
    isGrid = true;
    rows = 0;
    columns = 0;
    encoders = 0;
    //clearQueue();
    clearAllLeds();
    arcDirty = false;
    gridDirty = false;
}

void MonomeSerialDevice::setupAsGrid(uint8_t _rows, uint8_t _columns) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = true;
    rows = _rows;
    columns = _columns;
    gridDirty = true;
    debugfln(INFO, "GRID rows: %d columns %d", rows, columns);
}

void MonomeSerialDevice::setupAsArc(uint8_t _encoders) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = false;
    encoders = _encoders;
    arcDirty = true;
    debugfln(INFO, "ARC encoders: %d", encoders);
}

void MonomeSerialDevice::getDeviceInfo() {
    //debugln(INFO, "MonomeSerialDevice::getDeviceInfo");
    Serial.write(uint8_t(0));
 }

void MonomeSerialDevice::poll() {
    //while (isMonome && Serial.available()) { processSerial(); };
    if (Serial.available()) {
      processSerial();
    }
    //Serial.println("processSerial");
}


void MonomeSerialDevice::setAllLEDs(int value) {
  for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = value;
}

void MonomeSerialDevice::setGridLed(uint8_t x, uint8_t y, uint8_t level) {
//    int index = x + (y * columns);   
//    if (index < MAXLEDCOUNT) leds[index] = level;

    if (x < columns && y < rows) {
      uint32_t index = y * columns + x;
      leds[index] = level;
    }
    //debugfln(INFO, "LED index: %d x %d y %d", index, x, y);
}
        
void MonomeSerialDevice::clearGridLed(uint8_t x, uint8_t y) {
    setGridLed(x, y, 0);
    //Serial.println("clearGridLed");
}

void MonomeSerialDevice::setArcLed(uint8_t enc, uint8_t led, uint8_t level) {
    int index = led + (enc << 6);
    if (index < MAXLEDCOUNT) leds[index] = level;
    //Serial.println("setArcLed");
}
        
void MonomeSerialDevice::clearArcLed(uint8_t enc, uint8_t led) {
    setArcLed(enc, led, 0);
    //Serial.println("clearArcLed");
}

void MonomeSerialDevice::clearAllLeds() {
    for (int i = 0; i < MAXLEDCOUNT; i++) leds[i] = 0;
    //Serial.println("clearAllLeds");
}

void MonomeSerialDevice::clearArcRing(uint8_t ring) {
    for (int i = ring << 6, upper = i + 64; i < upper; i++) leds[i] = 0;
    //Serial.println("clearArcRing");
}

void MonomeSerialDevice::refreshGrid() {
    gridDirty = true;
    //Serial.println("refreshGrid");
}

void MonomeSerialDevice::refreshArc() {
    arcDirty = true;
    //Serial.println("refreshArc");
}

void MonomeSerialDevice::refresh() {
/*
    uint8_t buf[35];
    int ind, led;

    if (gridDirty) {
        //Serial.println("gridDirty");
        buf[0] = 0x1A;
        buf[1] = 0;
        buf[2] = 0;

        ind = 3;
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        Serial.write(buf, 35);
        
        ind = 3;
        buf[1] = 8;
        for (int y = 0; y < 8; y++)
            for (int x = 8; x < 16; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        Serial.write(buf, 35);
        
        ind = 3;
        buf[1] = 0;
        buf[2] = 8;
        for (int y = 8; y < 16; y++)
            for (int x = 0; x < 8; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        Serial.write(buf, 35);

        ind = 3;
        buf[1] = 8;
        for (int y = 8; y < 16; y++)
            for (int x = 8; x < 16; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        Serial.write(buf, 35);
        
        gridDirty = false;
    }

    if (arcDirty) {
        //Serial.print("arcDirty");
        buf[0] = 0x92;

        buf[1] = 0;
        ind = 2;
        for (led = 0; led < 64; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);
        
        buf[1] = 1;
        ind = 2;
        for (led = 64; led < 128; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);

        buf[1] = 2;
        ind = 2;
        for (led = 128; led < 192; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);
        
        buf[1] = 3;
        ind = 2;
        for (led = 192; led < 256; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);
        
        buf[1] = 4;
        ind = 2;
        for (led = 256; led < 320; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);

        buf[1] = 5;
        ind = 2;
        for (led = 320; led < 384; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);

        buf[1] = 6;
        ind = 2;
        for (led = 384; led < 448; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);

        buf[1] = 7;
        ind = 2;
        for (led = 448; led < 512; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        Serial.write(buf, 34);

        arcDirty = 0;
    }
*/
}

void MonomeSerialDevice::processSerial() {
    uint8_t identifierSent;  // command byte sent from controller to matrix
    uint8_t index, readX, readY, readN, readA;
    uint8_t dummy, gridNum, deviceAddress;  // for reading in data not used by the matrix
    uint8_t n, x, y, z, i;
    uint8_t intensity = 15;
    uint8_t gridKeyX;
    uint8_t gridKeyY;
    int8_t delta;
    uint8_t gridX    = columns;          // Will be either 8 or 16
    uint8_t gridY    = rows;
    uint8_t numQuads = columns/rows;
    
    // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
    // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
    // b = command (ie. query, enable, led, key, frame)

    identifierSent = Serial.read();  
    
    switch (identifierSent) {
        case 0x00:  // device information
        	// [null, "led-grid", "key-grid", "digital-out", "digital-in", "encoder", "analog-in", "analog-out", "tilt", "led-ring"]
            //Serial.println("0x00 system / query ----------------------");
            Serial.write((uint8_t)0x00); // action: response, 0x00 = system
            Serial.write((uint8_t)0x01); // section id, 1 = led-grid, 2 = key-grid, 5 = encoder/arc
            Serial.write((uint8_t)numQuads);   // one Quad is 64 buttons

            Serial.write((uint8_t)0x00); // send again with 2 = key-grid
            Serial.write((uint8_t)0x02); // 
            Serial.write((uint8_t)numQuads); 

            break;

        case 0x01:  // system / ID
            Serial.write((uint8_t)0x01);        // action: response, 0x01
            for (i = 0; i < 32; i++) {          // has to be 32
                if (i < deviceID.length()) {
                  Serial.write(deviceID[i]);
                } else {
                  Serial.write((uint8_t)0x00);
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
            gridNum = Serial.read();        // grid number
            readX = Serial.read();          // x offset
            readY = Serial.read();          // y offset 
            break;

        case 0x05:  // _SYS_GET_GRID_SIZE
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
            deviceAddress = Serial.read();     // I2C set addr - ignored
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
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8

          readY = Serial.read();                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8

          for (y = 0; y < 8; y++) {               // each i will be a row
            intensity = Serial.read();            // read one byte of 8 bits on/off
    
            for (x = 0; x < 8; x++) {             // for 8 LEDs on a row
              if ((intensity >> x) & 0x01) {      // if intensity bit set, light led full brightness
                setGridLed(readX + x, readY + y, 15); 
              }
              else {
                setGridLed(readX + x, readY + y, 0); 
              }
            }
          }
          break;

        case 0x15:                                //  /prefix/led/row x y d
          readX = Serial.read();                      // led-grid / set row
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8

          readY = Serial.read();                      // 
          intensity = Serial.read();                  // read one byte of 8 bits on/off

          for (x = 0; x < 8; x++) {               // for the next 8 lights in row
            if ((intensity >> x) & 0x01) {        // if intensity bit set, light led full brightness
              setGridLed(readX + x, readY, 15);
            } else {
              setGridLed(readX + x, readY, 0);
            }
          }

          break;

        case 0x16:                                //  /prefix/led/col x y d
          readX = Serial.read();                      // led-grid / column set

          readY = Serial.read();
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8

          intensity = Serial.read();                  // read one byte of 8 bits on/off

          for (y = 0; y < 8; y++) {               // for the next 8 lights in column
            if ((intensity >> y) & 0x01) {        // if intensity bit set, light led full brightness
              setGridLed(readX, readY + y, 15);
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
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          readY = Serial.read();                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          
          z = 0;
          for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
              if (z % 2 == 0) {                    
                intensity = Serial.read();
                if ( ((intensity >> 4) & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX + x, readY + y, (intensity >> 4) & 0x0F);
                } else {
                  setGridLed(readX + x, readY + y, 0);
                }
              } else { 
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX + x, readY + y, intensity & 0x0F);
                } else {
                  setGridLed(readX + x, readY + y, 0);
                }
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
          readX = Serial.read();                      // x offset
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          readY = Serial.read();                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          for (x = 0; x < 8; x++) {
              if (x % 2 == 0) {                    
                intensity = Serial.read();
                if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX + x, readY, (intensity >> 4) & 0x0F);
                }
                else {
                  setGridLed(readX + x, readY, 0);
                }
              } else {                              
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX + x, readY, intensity & 0x0F);
                }
                else {
                  setGridLed(readX + x, readY, 0);
                }
              }
          }
          break;

        case 0x1C:                                // /prefix/led/level/col x y d[8]
          readX = Serial.read();                      // x offset
          while (readX > 16) { readX += 16; }         // hacky shit to deal with negative numbers from rotation
          readX &= 0xF8;                              // floor the offset to 0 or 8
          readY = Serial.read();                      // y offset
          while (readY > 16) { readY += 16; }         // hacky shit to deal with negative numbers from rotation
          readY &= 0xF8;                              // floor the offset to 0 or 8
          for (y = 0; y < 8; y++) {
              if (y % 2 == 0) {                    
                intensity = Serial.read();
                if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                  setGridLed(readX, readY + y, (intensity >> 4) & 0x0F);
                }
                else {
                  setGridLed(readX, readY + y, 0);
                }
              } else {                              
                if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                  setGridLed(readX, readY + y, intensity & 0x0F);
                }
                else {
                  setGridLed(readX, readY + y, 0);
                }
              }
          }
          break;



    // 0x20 and 0x21 are for a Key inputs (grid) - see readKeys() function

        case 0x20:
            /*
             0x20 key-grid / key up
             bytes: 3
             structure: [0x20, x, y]
             description: key up at (x,y)
             */

            gridKeyX = Serial.read();
            gridKeyY = Serial.read();
            addGridEvent(gridKeyX, gridKeyY, 0);
            /*
            Serial.print("grid key: ");
            Serial.print(gridKeyX);
            Serial.print(" ");
            Serial.print(gridKeyY);
            Serial.print(" up - ");
            */
            break;
            
        case 0x21:
            /*
             0x21 key-grid / key down
             bytes: 3
             structure: [0x21, x, y]
             description: key down at (x,y)
             */
            gridKeyX = Serial.read();
            gridKeyY = Serial.read();
            addGridEvent(gridKeyX, gridKeyY, 1);
            /*
            Serial.print("grid key: ");
            Serial.print(gridKeyX);
            Serial.print(" ");
            Serial.print(gridKeyY);
            Serial.print(" dn - ");
            */
            break;

        // 0x5x are encoder
        case 0x50:
            // bytes: 3
            // structure: [0x50, n, d]
            // n = encoder number
            //  0-255
            // d = delta
            //  (-128)-127 (two's comp 8 bit)
            // description: encoder position change

            index = Serial.read();
            delta = Serial.read();
            addArcEvent(index, delta);
            /*
            Serial.print("Encoder: ");
            Serial.print(index);
            Serial.print(" : ");
            Serial.print(delta);
            Serial.println();
            */
            break;

        case 0x51:  // /prefix/enc/key n (key up)
            // Serial.println("0x51");
            n = Serial.read();
            /*
            Serial.print("key: ");
            Serial.print(n);
            Serial.println(" up");
            */
            // bytes: 2
            // structure: [0x51, n]
            // n = encoder number
            //  0-255
            // description: encoder switch up
            break;

        case 0x52:  // /prefix/enc/key n (key down)
            // Serial.println("0x52");
            n = Serial.read();
            /*
            Serial.print("key: ");
            Serial.print(n);
            Serial.println(" down");
            */
            // bytes: 2
            // structure: [0x52, n]
            // n = encoder number
            //  0-255
            // description: encoder switch down
            break;

        case 0x80:  //   tilt / active response - 9 bytes [0x01, d]
            break;
        case 0x81:  //   tilt - 8 bytes [0x80, n, xh, xl, yh, yl, zh, zl]
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

void MonomeEventQueue::addGridEvent(uint8_t x, uint8_t y, uint8_t pressed) {
    if (gridEventCount >= MAXEVENTCOUNT) return;
    uint8_t ind = (gridFirstEvent + gridEventCount) % MAXEVENTCOUNT;
    gridEvents[ind].x = x;
    gridEvents[ind].y = y;
    gridEvents[ind].pressed = pressed;
    gridEventCount++;
}

bool MonomeEventQueue::gridEventAvailable() {
    return gridEventCount > 0;
}

MonomeGridEvent MonomeEventQueue::readGridEvent() {
    if (gridEventCount == 0) return emptyGridEvent;
    gridEventCount--;
    uint8_t index = gridFirstEvent;
    gridFirstEvent = (gridFirstEvent + 1) % MAXEVENTCOUNT;
    return gridEvents[index];
}

void MonomeEventQueue::addArcEvent(uint8_t index, int8_t delta) {
    if (arcEventCount >= MAXEVENTCOUNT) return;
    uint8_t ind = (arcFirstEvent + arcEventCount) % MAXEVENTCOUNT;
    arcEvents[ind].index = index;
    arcEvents[ind].delta = delta;
    arcEventCount++;
}

bool MonomeEventQueue::arcEventAvailable() {
    return arcEventCount > 0;
}

MonomeArcEvent MonomeEventQueue::readArcEvent() {
    if (arcEventCount == 0) return emptyArcEvent;
    arcEventCount--;
    uint8_t index = arcFirstEvent;
    arcFirstEvent = (arcFirstEvent + 1) % MAXEVENTCOUNT;
    return arcEvents[index];
}

void MonomeEventQueue::sendArcDelta(uint8_t index, int8_t delta) {
    /*
    Serial.print("Encoder:");
    Serial.print(index);
    Serial.print(" ");
    Serial.print(delta);
    Serial.println(" ");
    */
    Serial.write((uint8_t)0x50);
    Serial.write((uint8_t)index);
    Serial.write((int8_t)delta);
    /*
    byte buf[3];
    buf[0] = 0x50;
    buf[1] = index;
    buf[2] = delta;
    Serial.write(buf, sizeof(buf));
    */
}

void MonomeEventQueue::sendArcKey(uint8_t index, uint8_t pressed) {
    /*
    Serial.print("key:");
    Serial.print(index);
    Serial.print(" ");
    Serial.println(pressed);
    */
    uint8_t buf[2];
    if (pressed == 1){
      buf[0] = 0x52;
    }else{
      buf[0] = 0x51;
    }
    buf[1] = index;
    Serial.write(buf, 2);
}

void MonomeEventQueue::sendGridKey(uint8_t x, uint8_t y, uint8_t pressed) {    
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
