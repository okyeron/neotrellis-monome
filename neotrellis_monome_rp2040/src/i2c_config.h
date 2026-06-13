#include <stdint.h> 

// I2C CONFIG 

#undef I2C_SDA
#undef I2C_SCL

#define PICO 1
#define KB2040QT 2
#define FEATHER2040QT 3

// DEFINE WHAT BOARD YOU ARE USING
// PICO OR KB2040QT or FEATHER2040QT
// change platformio.ini board to use `adafruit_feather`
#ifndef BOARDTYPE
#define BOARDTYPE PICO
#endif

// SET YOUR ADDRESSES 
// 16x8
const uint8_t addrRowOne[4] = {0x32,0x30,0x2F,0x2E}; 
const uint8_t addrRowTwo[4] = {0x33,0x31,0x3E,0x36}; 

// 8x8
// const byte addrRowOne[4] = {0x2F,0x2E}; 
// const byte addrRowTwo[4] = {0x32,0x30}; 

 
// KeeBoar KB2040 - STEMMA-QT uses 12/13 and Wire
#if BOARDTYPE == KB2040QT
  #define MYWIRE Wire
  #define I2C_SDA 12
  #define I2C_SCL 13
#endif

// Feather RP2040 - STEMMA-QT uses 2/3 and Wire
#if BOARDTYPE == FEATHER2040QT
  #define MYWIRE Wire
  #define I2C_SDA 2
  #define I2C_SCL 3
#endif

// DEFAULT FOR PICO
#ifndef I2C_SDA
#define I2C_SDA 26
#endif
#ifndef I2C_SCL
#define I2C_SCL 27
#endif
#ifndef MYWIRE
#define MYWIRE Wire1
#endif