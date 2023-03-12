# neotrellis monome compatible grid

Neotrellis Grid Code for Adafruit M0 and M4 controllers.

This code should work with any of the various Adafruit SAMD21/51 M0/M4 controllers such as Feather, ItsyBitsy, etc. 

__NOTE__ - You will need to recompile the code for the specific board you are using. Compiled firmwares such as `neotrellis_m4_8x8_featherM4.UF2` will ONLY work on that specific board type. 

Step 1: Be sure update to the most recent TinyUSB library version. If you're using Arduino, it's best to use Manage Libraries in Arduino to install/update TinyUSB.

Step 2: Be sure you check the board addresses for your build and adjust the code as needed if you do something different than the default described here.


## Adafruit Feather M4 Kit
[Adafruit 8x8 NeoTrellis Feather M4 Kit Pack](https://www.adafruit.com/product/1929)

If you have this (or have built an 8x8 grid with Feather M4) you can use the enclosed compiled firmware [neotrellis_m4_8x8_featherM4.UF2](neotrellis_m4_8x8_featherM4.UF2) assuming your setup uses the following addresses 
```
{ Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x30) },
{ Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x36) }

```
