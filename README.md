# neotrellis monome compatible grid

**Status**: work in progress. Not exactly noob friendly just yet.

## What is it?

Code use a set of [Adafruit NeoTrellis boards](https://www.adafruit.com/product/3954) as a monome grid clone.

Mostly tested using a Teensy 3.2 microcontroller. 

There is code for using an Adafruit ItsyBitsy M0 (and by extension the Feather M0/M4), but this requires some changes to the underlying libraries (replacing the Adafruit_USBD_Device library deep in the adafruit/arduino core libraries).


## norns / fates / norns shield

The norns software requires some hacking/changes to get a DIY grid working. More on this later


## references

### mext / monome serial protocol

The mext protocol is used for serial communication - same as what is used in most recent monome devices.

### serialosc / libmonome

`serialosc` is required for serial communication with MacOS/Windows computers and OSC devices. Linux may use `serialosc` or `libmonome` depending on the application. `libmonome` is basically driver code which also facillitates monome serial communication.

You can get both `serialosc` and `libmonome` code from [monome github page](https://github.com/monome), and building them is well documented on official linux docs (they work for macOS as well), read part *2 Preparing your system: serialosc* (ignoring the `sudo apt-get` - I was missing `liblo`, but it's available on homebrew): [monome.org/docs/linux/](https://monome.org/docs/linux/).


## Case / Panel

Work in progress