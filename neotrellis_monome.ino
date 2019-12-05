#include <Adafruit_NeoTrellis.h>
#include "MonomeSerialDevice.h"


#define DIM_X 4
#define DIM_Y 4
#define INT_PIN 9
#define BRIGHTNESS 50

// Monome setup
#define MONOMEDEVICECOUNT 1

MonomeSerial monomeDevices;
//monomeDevices->isMonome = true;
//monomeDevices->setupAsGrid(8, 16);
//elapsedMillis monomeRefresh;


Adafruit_NeoTrellis trellis_array[DIM_Y / 4][DIM_X / 4] = {
    { Adafruit_NeoTrellis(0x2F) }
/*    , Adafruit_NeoTrellis(0x2F),Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x31)},
    { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x33),Adafruit_NeoTrellis(0x34), Adafruit_NeoTrellis(0x35) }*/
};
     
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, DIM_Y / 4, DIM_X / 4);

boolean *lit_keys;

String deviceID = "m128000000";

uint32_t ledBuffer[DIM_X][DIM_Y];
uint32_t prevReadTime = 0;
uint32_t prevWriteTime = 0;
uint8_t currentWriteX = 0;

// ***************************************************************************
// **                         FUNCIONES PARA TRELLIS                        **
// **                          FUNCTIONS FOR TRELLIS                        **   
// ***************************************************************************

void setLED(uint8_t x, uint8_t y, uint32_t value) { ledBuffer[x][y] = value; }

void setAllLEDs(uint32_t value) {
  uint8_t x, y;

  for (x = 0; x < DIM_X; x++) {
    for (y = 0; y < DIM_Y; y++) {
      ledBuffer[x][y] = value;
    }
  }
}

void turnOffLEDs() { setAllLEDs(0x000000); }
void turnOnLEDs() { setAllLEDs(0xFFFFFF); }

void PintarMatriz(){
	// unoptimised (show):
	{
		for (uint8_t x = 0; x < DIM_X; x++) {
		  for (uint8_t y = 0; y < DIM_Y; y++) {
			trellis.setPixelColor(x, y, ledBuffer[x][y]);
			//String valor = (String)ledBuffer[x][y];
			//Serial.println(String(x) + " " + (String)y + " = " + valor);       
		  }
		}
    	trellis.show();
	}
}

// NOT SURE THIS IS USED?

void processPushButton(){
    uint8_t x, y, i, keypad_count;

    if (!digitalRead(INT_PIN)) {
		for (x = 0; x < DIM_X / 4; x++) {
			for (y = 0; y < DIM_Y / 4; y++) {
			  keypad_count = trellis_array[y][x].getKeypadCount();

			  keyEventRaw e[keypad_count];
			  trellis_parts[y][x].readKeypad(e, keypad_count);

			  for (i = 0; i < keypad_count; i++) {
				uint8_t xx = e[i].bit.NUM % 4;
				uint8_t yy = e[i].bit.NUM / 8;
   
				if (e[i].bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
				  writeInt(0x21);
				} else {
				  writeInt(0x20);
				}
				writeInt(x * 4 + xx);
				writeInt(y * 4 + yy); 
	
			  }
			}
		}
    }
}

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING)
    trellis.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, X_DIM*Y_DIM, 0, 255))); //on rising
  else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING)
    trellis.setPixelColor(evt.bit.NUM, 0); //off falling
    
  trellis.show();
  return 0;
}


void setup(){
	Serial.begin(115200);
    
	if(!trellis.begin()){
		Serial.println("failed to begin trellis");
		while(1);
	}
	//trellis.setBrightness(BRIGHTNESS);

	Serial.println("--NeoTrellis Monome--");

// WHAT'S THIS ABOUT?
	lit_keys = new boolean[trellis.num_keys()];  
	for (int i=0; i<trellis.num_keys(); i++) {
		lit_keys[i] = false;
	}
	
	// Configura la matrix con brillo inicial.
	// full brightness bricks arduino, probably a current thing
	uint8_t x, y;
	for (x = 0; x < DIM_X / 4; x++) {
		for (y = 0; y < DIM_Y / 4; y++) {
		  trellis_parts[y][x].pixels.setBrightness(BRIGHTNESS);  // DOES THIS NEED TO HAPPEN FOR EACH PIXEL?
		}
	}
	for (x = 0; x < DIM_X; x++) {
		for (y = 0; y < DIM_Y; y++) {
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
		  trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
		}
	}
	// Declara el pin de interrupción (indica que se ha pulsado algún botón)
	pinMode(INT_PIN, INPUT);

	turnOffLEDs();
}

// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {
	unsigned long now = millis();
	// put your main code here, to run repeatedly:
	trellis.tick();

	while(trellis.available()){
		keypadEvent e = trellis.read();
		Serial.print((int)e.bit.KEY);
		if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
			Serial.println(" pressed");
			// press stuff
			monomeDevices.sendGridKey(x, y, 1);

			trellis.setPixelColor(e.bit.KEY, 0xFFFFFF);
		} else if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
			Serial.println(" released");
			// release stuff
			monomeDevices.sendGridKey(x, y, 0);

			trellis.setPixelColor(e.bit.KEY, 0x0);
		}
	}
  
	//delay(10);
}
