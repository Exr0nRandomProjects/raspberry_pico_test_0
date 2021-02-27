# Raspberry Pico Test

## CLI

### Toolchain Install

Make sure to clone the [pico-sdk](https://github.com/raspberrypi/pico-sdk) into the parent folder of this one.

Then, install things until building works. Probably need atleast
```
brew install --cask gcc-arm-embedded
brew install cmake
```

[gcc-arm-embedded trick source](https://gist.github.com/joegoggins/7763637).

### Build

Then, just `make`.

### Debug pico
Unplug, hold BOOTSEL, replug. Then, `sudo picotool info -a`.

## Links
- [api documentation](https://raspberrypi.github.io/pico-sdk-doxygen/index.html)
- [pinout summary](https://microcontrollerslab.com/raspberry-pi-pico-pinout-features-programming-peripherals/)
- [detailed engineering analysis of the SDK](https://www.stereorocker.co.uk/2021/02/14/raspberry-pi-pico-displays-fonts-portability/)
- [tools for reading printf output via usb](https://www.raspberrypi.org/forums/viewtopic.php?t=302227)

