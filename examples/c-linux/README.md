## I2LCD Userspace C library

Here is I2LCD library written in C for Linux. Library use exsiting I2C device
so the modules for I2C bus should be loaded into kernel prior to use this library.
No external libraries were linked, library use Linux API entirely.

Main directory contains few examples, i2lcd subdirectory contains source code of
the library.

## Examples:

* editlcd - LCD content editor. You can edit content of the display in realtime in
  convenient window. Editor also allow to edit graphical characters and save its
  definiton to C header file.


* lcdbricontest - simple test using brighness and contrast manipulation which shows
  current value of those parameters on the display.

* lcdtest - simple test of the display

## Library - i2lcd subdirectory

* i2lcd.c - main library source file
* i2lcd.h - header for i2lcd.c
* pca9535.c - source of api for PCA9535 chip used by i2lcd.c
* pca9535.h - header file for the above
* pots.c - source of api of the potentiometers used in the module
* pots.h - header for pots.c api

