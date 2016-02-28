## I2LCD - I2C HD4470 expansion board with software set contrast and backlight

Here's little project of electronic module for ubiquitous Hitachi HD4470 based
alphanumeric displays. Main reason for making the project was to drive the
displays using I2C bus. Of course there are many existing I2C extension boards
built upon NXP PCF8574 8 bit port expander. This module however, provide few
additional functions.
PCB of the module was made in KiCAD 4.0. Generated gerber
files were sent to the PCD manufacturer and after few minor modifications and
few weeks of waiting, PCB of a prototype were sent back. Here are the results.

What's the difference.

- *Full 8 bit communication with the display*.
  It uses PCA9535 16 bit expander, so we have spare IO ports to
communicate using full 8 bit width words. In popular I2C expansion modules
4 bits are used to drive control lines of the display and 4 other bits are used
as data lines. This module project, was made to **provide full 8 bit communication**.
No need to send data bytes as two 4 bit chunks although sent as 8 bits of which
4 bits are meaningful and 4 other are just ignored.

- *You can read data from the display*.
  Not all I2C modules provide ability to read from the display. Some of
implementations have hard-wired R/W line assuming user will always write to the
display. **Using this module you are able to read data from the module**. Attached example
libraries use this feature to read the busy flag of the display. No longer
arbitrary delays in your code, your code waits as long as it is required for the
display to finish.

- *Contrast and backlight intensity are handled programmatically*.
  Another eight IO lines are used for driving control lines of a display and
changing backlight intensity, setting contrast and other functions.
Usually I2C expansion modules use a potentiometer to set contrast of a
display, depending on the module, contrast setting may differ, so you take
a screwdriver and change the pot until the display shows its content with
desired contrast. **The module has two electronic potentiometers to set
the contrast and for changing backlight intensity**. Where on popular I2C
modules you're able to switch backlight on or off. Potentiometers use simple
Up/Down communication protocol.


- *You can programmatically switch on or off the display power supply*.
Additional line is used to **switch the display power on or off**. If for some reason
the display should be switched off (to save battery usage for example), you can
do it from your code. However there are some issues regarding this version of
module. Read this Wiki article about known issues and how to get rid of them.

