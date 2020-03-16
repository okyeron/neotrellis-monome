# neotrellis monome compatible grid

**Status**: work in progress. Not exactly noob friendly just yet.

## What is it?

Code to use a set of [Adafruit NeoTrellis boards](https://www.adafruit.com/product/3954) as a monome grid clone.

Tested mostly using a Teensy 3.2 microcontroller. 

There is code for using an Adafruit ItsyBitsy M0 (and by extension the Feather M0/M4), but this requires some changes to the underlying libraries (replacing the Adafruit_USBD_Device library deep in the adafruit/arduino core libraries).

Compiled firmware for Feather M4 and ItsyBitsy M0 coming soon.

## norns / fates / norns shield

The norns software requires some hacking/changes to get a DIY grid working. More on this later

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
