#include "NeoTrellisM4.h"


static const byte NEO_PIN = 10;

static const byte ROWS = 4; // four rows
static const byte COLS = 8; // eight columns

//define the symbols on the buttons of the keypads
static byte trellisKeys[ROWS][COLS] = {
  {0, 1,  2,  3,  4,  5,  6,  7},
  {8, 9,  10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31}
};
static byte rowPins[ROWS] = {14, 15, 16, 17}; //connect to the row pinouts of the keypad
static byte colPins[COLS] = {2, 3, 4, 5, 6, 7, 8, 9}; //connect to the column pinouts of the keypad

NeoTrellisM4::NeoTrellisM4(void) :
  Adafruit_Keypad(makeKeymap(trellisKeys), rowPins, colPins, ROWS, COLS),
  Adafruit_NeoPixel_ZeroDMA(ROWS*COLS, NEO_PIN, NEO_GRB)
{
  _num_keys = ROWS * COLS;
  _rows = ROWS;
  _cols = COLS;
  _auto_update = true;
}

void NeoTrellisM4::begin(void) {
  Adafruit_Keypad::begin();

  // Initialize all pixels to 'off'
  Adafruit_NeoPixel_ZeroDMA::begin();
  fill(0x0);
  Adafruit_NeoPixel_ZeroDMA::show();
  Adafruit_NeoPixel_ZeroDMA::setBrightness(255);
}

void NeoTrellisM4::setPixelColor(uint32_t pixel, uint32_t color) {
  Adafruit_NeoPixel_ZeroDMA::setPixelColor(pixel, color);
  if (_auto_update) {
    Adafruit_NeoPixel_ZeroDMA::show();
  }
}

void NeoTrellisM4::autoUpdateNeoPixels(boolean flag) {
  _auto_update = flag;
}

void NeoTrellisM4::fill(uint32_t color) {
  for (int i=0; i<ROWS*COLS; i++) {
    Adafruit_NeoPixel_ZeroDMA::setPixelColor(i, color);
  }
  if (_auto_update) {
    Adafruit_NeoPixel_ZeroDMA::show();
  }
}

void NeoTrellisM4::tick(void)
{
  Adafruit_Keypad::tick();
  // look for an entire column being pressed at once and if it was, clear the whole buffer
  uint8_t rcount[] = {0, 0, 0, 0, 0, 0, 0, 0};
  for(int i=0; i<(COLS*ROWS)-1; i++){
    if (Adafruit_Keypad::justPressed(i+1, false))
      rcount[i%COLS]++;
  }
  for (int i=0; i<COLS; i++){
    if (rcount[i] >= ROWS){
      Adafruit_Keypad::clear();
      break;
    }
  }
}




