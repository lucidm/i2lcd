## I2LCD Userspace Python library

Here is I2LCD library written in Pyhton for Linux. Library is using exsiting I2C device
so the modules for I2C bus should be loaded into kernel prior to use this library.
No external libraries were used, library use Linux API entirely.

Main directory contains few examples, i2lcd subdirectory contains source code of
the library.

## Examples:

* lcdtest - simple test of the display

## Library - i2lcd subdirectory

* i2lcd.py - main library source file
* lcdconst.py - constants used by the library
* pca9535.py - source of api for PCA9535 chip used by i2lcd.py
* pots.py - source of api of the potentiometers used in the module
* lcdstatusflag.py - classes for different method of maintaining status flags
  of the display

