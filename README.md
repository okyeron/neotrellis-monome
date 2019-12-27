# neotrellis monome compatible grid

**Status**: work in progress. Not exactly noob friendly just yet.

## What is it?

Code use a set of [Adafruit NeoTrellis boards](https://www.adafruit.com/product/3954) as a monome grid clone.

Mostly tested using a Teensy 3.2 microcontroller. 

There is code for using an Adafruit ItsyBitsy M0 (and by extension the Feather M0/M4), but this requires some changes to the underlying libraries (replacing the Adafruit_USBD_Device library deep in the adafruit/arduino core libraries).


## norns / fates / norns shield

The norns software requires some hacking/changes to get a DIY grid working. More on this later

## neotrellis building/testing notes

* Review the [Adafruit tutorial on the neotrellis](https://learn.adafruit.com/adafruit-neotrellis/arduino-code) boards. NOTE - there's a typo on that page "the address would be 0x2E + 1 + 2 = 0x30." That address would be 0x31

* Don't worry about the INT pin - it's not used in the grid software.

* Be sure you have the Adafruit Seesaw libraries installed (via the Arduino Library Manager)

* Try to test each neotrellis board individually using the Adafruit examples in the File>Examples>Adafruit seesaw Library>Neotrellis menu. The Basic sketch is good for testing individual boards. The multitrellis>basic sketch is good once you have all the boards connected and addresses assigned.

* [neotrellis i2c address chart](./NeoTrellis Addresses.txt) (for soldering the jumpers on each neotrellis board)

* The multitrellis array will fail to init if the addresses are wrong, or the wrong number of boards are attached.

* The Teensy specific i2c_t3 examples can be used to double check your i2c addresses. See File>Examples>i2c_t3>basic_scanner for more.

## references

### mext / monome serial protocol

The mext protocol is used for serial communication - same as what is used in most recent monome devices.

### serialosc / libmonome

`serialosc` is required for serial communication with MacOS/Windows computers and OSC devices. Linux may use `serialosc` or `libmonome` depending on the application. `libmonome` is basically driver code which also facillitates monome serial communication.

You can get both `serialosc` and `libmonome` code from [monome github page](https://github.com/monome), and building them is well documented on official linux docs (they work for macOS as well), read part *2 Preparing your system: serialosc* (ignoring the `sudo apt-get` - I was missing `liblo`, but it's available on homebrew): [monome.org/docs/linux/](https://monome.org/docs/linux/).


## Case / Panel

Work in progress