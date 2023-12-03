# MathiasStraws

## OVERVIEW:
The intention of this project is to create a bluetooth keyboard/gamepad emulator that will serve as a gaming device for Mathias, a 14 year old boy and recent quadruple amputee. The project is intended for an ESP32 WROOM microcontroller. 

## RESOURCES:
pinouts:
https://www.circuitstate.com/pinouts/doit-esp32-devkit-v1-wifi-development-board-pinout-diagram-and-reference/#Pinout_Diagram

USB Serial Driver download/install:
https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads

## SETUP:

Use VSCode with PlatformIO installed or another IDE for Microchip booting. Ensure your computer has a USB Serial com port using device manager on windows, or the equivalent on other OS (look at COM6) below:
![image](https://github.com/zawatse/MathiasStraws/assets/35353895/ffe3da15-8b3c-4a13-974c-bfbd9149af57)

NOTE: You will need to identify your USB-TO-UART bridge chip, and download the correlating driver for that model chip. The following link was helpful in this process: https://bromleysat.com/installing-drivers-for-the-esp32

If it does not, use the link above to isntall a driver.

TODO: hardware setup instructions for default code configuration

## KNOWN ISSUES

We use both the h2zero/NimBLE-Arduino lib and the Arduino ESP32 BLE lib, and these are currently incompatible. I've opened an issue with h2zero/NimBLE-Arduino to create a permanent fix, but for now, the workaround is to navigate to your local copy of NimBLE-Arduino\src\NimBLEHIDDevice.h and change the following lines:

#ifndef _BLEHIDDEVICE_H_

#define _BLEHIDDEVICE_H_

to

#ifndef _NIMBLEHIDDEVICE_H_

#define _NIMBLEHIDDEVICE_H_





