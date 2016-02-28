## I2LCD Userspace C++ library

Library for I2LCD module written in C++ for Linux. Library use exsiting I2C device
so the modules for I2C bus should be loaded into the kernel prior to use this library.
No external libraries were linked, library use Linux API entirely. Library defines
I2Lcd class with api methods to control the display. Whole definition was i2lcd
namespace enclosed. Only the '<<' operator is defined outside of the namespace,
to provide ability to dump content of the display to ostream object.

Main directory contains few examples and library itself.

## Examples:

* lcdtest - simple test of the display

## The library

* i2lcd.cpp - main library source file, I2Lcd
* i2lcd.h - header for i2lcd.c
* pca9535.cpp - source of PCA9535 class with its API
* pca9535.h - header file for the above
* pots.cpp - source of Potentiometer class and its API
* pots.h - header for pots.cpp api

