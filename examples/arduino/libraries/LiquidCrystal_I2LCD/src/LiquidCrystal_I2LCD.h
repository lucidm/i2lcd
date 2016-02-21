// ---------------------------------------------------------------------------
// Created by Jarek Zok on 25/01/2016.
// Copyright 2016 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal_I2LCD.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK but using an I2C IO extension board.
//
// @brief
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using I2C extension
// with the PCA9535 I2C IO Expander ASIC.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
//
// @author J. Zok - jarekzok@gmail.com
// ---------------------------------------------------------------------------
#ifndef __LIQUIDCRYSTAL_I2LCD_H__
#define __LIQUIDCRYSTAL_I2LCD_H__
#include <inttypes.h>
#include "Print.h"

#include "PCA9535.h"

//Entry Mode Set functions
#define EMS_S	1
#define EMS_ID	(1 << 1)

//Dispaly On - Off functions
#define DOO_B	1
#define DOO_C	(1 << 1)
#define DOO_D	(1 << 2)

//Cursor Display Shift functions
#define CDS_RL	(1 << 2)
#define CDS_SC	(1 << 3)

//Function Set - functions
#define FS_F	(1 << 2)
#define FS_N	(1 << 3)
#define FS_DL	(1 << 4)

//Pins definitions of Control Port
#define	EN	1
#define RW	(1 << 1)
#define RS	(1 << 2)
#define IRS	(1 << 3)
#define PWR	(1 << 4)
#define UD      (1 << 5)
#define CONTRAST_CS (1 << 6)
#define BACKLIGHT_CS (1 << 7)

//Busy bit definition of Data Port
#define BUSY_FLAG   (1 << 7)

// flags for function set
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

//Values of t_LCDType are calculated by interleaving bits of width and height of
//the display (see morton code)
enum t_LCDType {
    D6x1 = 22,
    D8x1 = 66,
    D8x2 = 72,
    D12x4 = 122,
    D16x0 = 256,
    D16x1 = 258,
    D16x2 = 264,
    D16x4 = 288,
    D20x2 = 280,
    D20x4 = 304,
    D24x2 = 328,
    D40x2 = 1096,
};

enum t_Command {
    CLEAR_DISPLAY=0,
    CURSOR_HOME,
    ENTRY_MODE_SET,
    DISPLAY_ONOFF,
    CURSOR_DISPLAY_SHIFT,
    FUNCTION_SET,
    SET_CGRAM_ADDRESS,
    SET_DDRAM_ADDRESS,
};

class LiquidCrystal_I2LCD : public PCA9535, public Print
{
using Print::write;
private:

    uint8_t address;
    uint8_t control;
    uint8_t contrast;
    uint8_t brightness;
    uint8_t columns;
    uint8_t rows;
    uint8_t column;
    uint8_t row;
    uint8_t charsize;
    uint8_t row_offsets[4];
    uint8_t commands[8];
    char buffer[40];
    t_LCDType type;

    bool waitflag;
    PCA9535 pcachip;

    //LCD communication methods
    void _offset(t_LCDType type);
    void _command(t_Command command, uint8_t value);
    void _control(uint8_t flags, bool value);
    uint8_t _status(void);
    void _usleep(uint32_t usdelay); //Probably not needed as the AVRs are
                                    //too slow for such short period.
                                    //But maybe it'll be better to leave it
                                    //for future compatibility.

    //Potentiometers methods
    void _ud(bool v);
    void _cs(uint8_t cspin, bool v);
    void _inc(uint8_t v);
    void _dec(uint8_t v);
    void _set(uint8_t v, uint8_t value);
    void _writeblock(const char *block, uint8_t len);
    void _readblock(char *block, uint8_t len);
    uint16_t _partbyte(uint16_t n);
    uint16_t _interleave(uint8_t width, uint8_t height);
    uint16_t _unpartbyte(uint16_t n);
    uint16_t _deinterleave(uint16_t n);


public:

   LiquidCrystal_I2LCD (uint8_t lcd_Addr);

   int init(void);
   void home(void);
   void clear(void);
   void on(void);
   void off(void);
   void noDisplay(void);
   void display(void);
   void noBlink(void);
   void blink(void);
   void noCursor(void);
   void cursor(void);
   void scrollDisplayLeft(void);
   void scrollDisplayRight(void);
   void leftToRight(void);
   void rightToLeft(void);
   void noAutoscroll(void);
   void autoscroll(void);
   void createChar(uint8_t, uint8_t[], uint8_t count = 8);
   void readChar(uint8_t location, uint8_t *charmap, uint8_t count = 8);
   void setCursor(uint8_t, uint8_t);
   void begin(t_LCDType lcd_Type = D16x2);
   t_LCDType begin(uint8_t col, uint8_t row);
   void setBacklight(uint8_t value);
   void setContrast(uint8_t value);
   virtual size_t write(uint8_t);
   char read(void);
   const char *read(uint8_t row);
   void writeCGRAM(uint8_t address, const char *buffer, uint8_t count);
   void readCGRAM(uint8_t address, char *buffer, uint8_t count);
};
#endif

