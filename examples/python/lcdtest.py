#!/usr/bin/env python
# -*- coding: utf-8 -*-

import i2lcd
import sys
import time
import random

if __name__ == "__main__":

    lcd = i2lcd.I2LCD(2, 0x20, 16, 1)
    lcd.power(1)
    lcd.clear()

    lcd.setCursor(0, 0)
    lcd.fprint("\x02\x02Hello Emy\x02\x02")

    lcd.setGC(0x02, [0b00000,
		     0b11011,
		     0b11111,
		     0b01110,
		     0b00100,
		     0b00000,
		     0b00000,
		     0b00000,
		    ])

    lcd.setBacklight(0x3f)
    lcd.setContrast(0x0c)
    lcd.blink(True)
    lcd.cursor(True)

    for i in range(lcd.rows):
	print lcd.readRow(i)

    time.sleep(3)

    lcd.power(0)
    lcd.close()
