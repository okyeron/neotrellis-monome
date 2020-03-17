# neotrellis monome compatible grid

**Status**: work in progress. Not exactly noob friendly just yet.

## What is it?

Code to use a set of [Adafruit NeoTrellis boards](https://www.adafruit.com/product/3954) as a monome grid clone.

Tested mostly using a Teensy 3.2 microcontroller. 

There is code for using an Adafruit ItsyBitsy M0 (and by extension the Feather M0/M4), but this requires some changes to the underlying libraries (replacing the Adafruit_USBD_Device library deep in the adafruit/arduino core libraries).

Compiled firmware for Feather M4 and ItsyBitsy M0 coming soon.

## compatibility

### Max or other computer-based applications

At the time of writing, this code works as expected with the neotrellis-grid connected to a computer with Max running monome patches.

### ansible/ trilogy modules

Does not work. Don’t ask (unless you know C well and can help me make changes to libavr32).

### norns /norns shield

Officially unsupported. Hacking required. Proceed at your own risk. May void your warranty. Prohibited in some states.

Unfortunately this code will not work right off the shelf with the stock norns codebase (norns and norns shield) due to some USB device management stuff.

But... there is a hack workaround, but it does require changes to the norns C code. It’s not hard, but steps will need to be repeated after updates, etc. Contact me for instructions.

###  Fates

For Fates devices , I have a script in my repo which will run the fix.

## BOM:  

8 [Adafruit NeoTrellis driver boards](https://www.adafruit.com/product/3954) [alt [Mouser](https://www.mouser.com/ProductDetail/485-3954) | [Digikey](https://www.digikey.com/products/en?keywords=1528-2712-ND) ]    

8 [Button keypads](http://www.adafruit.com/product/1611) [alt [Mouser](https://www.mouser.com/ProductDetail/485-1611) | [Digikey](https://www.digikey.com/products/en?keywords=1528-1559-ND) ]    

1 [Adafruit micro B USB Breakout](http://www.adafruit.com/product/1611) [alt [Mouser](https://www.mouser.com/ProductDetail/485-1833) | [Digikey](https://www.digikey.com/products/en?keywords=1528-1383-ND) ]    

1 [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) [alt [Adafruit](https://www.adafruit.com/product/2756) ]  


## before building

* Test each neotrellis board individually using the Adafruit examples in the `File>Examples>Adafruit seesaw Library>Neotrellis` menu. The Basic sketch is good for testing individual boards. The `multitrellis>basic` sketch is good once you have all the boards connected and addresses assigned.


## neotrellis building

* Review the [Adafruit tutorial on the neotrellis](https://learn.adafruit.com/adafruit-neotrellis/arduino-code) boards. 

* [see this video](https://www.youtube.com/watch?v=petILmGcNwQ) for an example of how to join the boards together

* [this graphic](neotrellis_addresses.jpg) shows a default layout of addresses and jumper positions for 8 neotrellis boards.

* new default address order in the code for 16x8 layout (__NOTE__ the address are in reverse order in the code - as compared to the graphic above):  
```
  { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)}, // top row
  { Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x3E), Adafruit_NeoTrellis(0x36) } // bottom row
```

* see this [neotrellis i2c address chart](NeoTrellis_Addresses.txt) if you want to define your own addresses

* Don't worry about the INT pin - it's not used in the grid software.

## troubleshooting / testing 

* Be sure you have the Adafruit Seesaw libraries installed and are up to date (via the Arduino Library Manager)

* Be aware - the multitrellis array will fail to initialize if the addresses are wrong, or the wrong number of boards are attached.

* There are Teensy specific i2c_t3 example sketches which can be used to double check your i2c addresses. See `File>Examples>i2c_t3>basic_scanner` for more.

* use [multitrellis_test](multitrellis_test/multitrellis_test.ino) sketch to test fully assembled grid before flashing neotrellis_monome_teensy.

## build help / support / troubleshooting

[see this thread](https://llllllll.co/t/diy-monome-compatible-grid-w-adafruit-neotrellis/28106?u=okyeron) on the lines forum for assistance.

## alternate firmware for color palettes 

https://github.com/oldmanfury/neotrellis-grid-paletted


## testing with serialosc on MacOS

If you already have serialosc installed/running, you may need to unload serialosc to get arduino to properly flash the teensy, then load serialosc to get Max to recognize the grid.  

`launchctl unload /Library/LaunchAgents/org.monome.serialosc.plist`  
`launchctl load /Library/LaunchAgents/org.monome.serialosc.plist`  

Max apps for testing:  
[Monome Home](https://github.com/monome-community/monome-home)  
[test-grid](https://github.com/monome/serialosc.maxpat)  


## references

### mext / monome serial protocol

The mext protocol is used for serial communication - same as what is used in most recent monome devices.

### serialosc / libmonome

`serialosc` is required for serial communication with MacOS/Windows computers and OSC devices. Linux may use `serialosc` or `libmonome` depending on the application. `libmonome` is basically driver code which also facillitates monome serial communication.

You can get both `serialosc` and `libmonome` code from [monome github page](https://github.com/monome), and building them is well documented on official linux docs (they work for macOS as well), read part *2 Preparing your system: serialosc* (ignoring the `sudo apt-get` - I was missing `liblo`, but it's available on homebrew): [monome.org/docs/linux/](https://monome.org/docs/linux/).


## Case / Panel

[Work in progress](<enclosure/README.md>)
