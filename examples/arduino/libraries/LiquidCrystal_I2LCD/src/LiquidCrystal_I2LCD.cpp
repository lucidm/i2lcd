// ---------------------------------------------------------------------------
// Created by Jarek Zok on 2016/01/28.
// Copyright 2016 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal_I2LCD.c
// This file implements a liquid crystal library using an I2C IO extension board.
//
// @brief
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using I2C extension
// backpack. You can find project and schematics at
//           http://github.com/lucidm/i2lcd
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
// Additionally provides reading capability of DDRAM and CGRAM content of the
// display and control of the backlight intensity and contrast as well.
//
//
//
// @author Jarek Zok - jarekzok@gmail.com
// ---------------------------------------------------------------------------
#include <Arduino.h>

#include <inttypes.h>
#include "LiquidCrystal_I2LCD.h"

const uint8_t potTransTable[64] = {0,10,16,21,24,27,29,31,33,35,36,37,39,40,41,42,43,44,44,45,46,47,47,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,55,56,56,57,57,57,58,58,58,59,59,59,60,60,60,60,61,61,61,62,62,62,62,63,63,63};

LiquidCrystal_I2LCD::LiquidCrystal_I2LCD (uint8_t lcd_Addr)
{
    address = lcd_Addr;
    brightness = 0x1f;
    contrast = 0x1f;
}

/**
 * @brief Native begin() method of the class. Instead col, row pair it
 * uses constant defining type of the display. Values of constants are NOT
 * arbitrary and are calculated using Morton code (aka Z-curve), so we can
 * implement overloaded begin() method of traditional form with col and row
 * parameters.
 *
 */
void LiquidCrystal_I2LCD::begin(t_LCDType lcd_Type)
{
    type = lcd_Type;
    _offset(type);
    waitflag = false;
    init(); // Initialise the I2C expander interface
    setBacklight(brightness);
    setContrast(contrast);
}

/**
 * @brief Traditional begin() method expecting columns and rows as input
 *
 * @param number of columns the display has
 * @param number of rows the display has
 * @return recognized type
 */
t_LCDType LiquidCrystal_I2LCD::begin(uint8_t col, uint8_t row)
{
    t_LCDType lcd_type = (t_LCDType)_interleave(col, row);
    begin(lcd_type);
    return lcd_type;
}

/**
 * @brief Switch power of display - on
 * As the display can be physically powered off, switching it on requires
 * full initialisation sequence, so internally we call begin() method with
 * current LCD type.
 **/
void LiquidCrystal_I2LCD::on(void)
{
    if (!(control & PWR)) //Do nothing if power is already switched on
    {
        control = control | PWR;
        setOutput(CPORT, control);
        begin(type);
    }
}

/**
 * @brief Switch power off
 * The display will be shut down, in this state it will draw no current.
 * As the backlight circuitry is nowhere connected to the rest of the
 * LCD board, backlight will draw current even when the the display is not
 * powered, you should set backlight potentiometer to minimum.
 * However expander chip will still be powered.
 **/
void LiquidCrystal_I2LCD::off(void)
{
    control = control & (~PWR);
    setOutput(CPORT, control);
}

/**
 * @brief Call internal LCD command "return home".
 * Cursor will appear in first column and row
 * Class row and column also will be set to
 * first row and column.
 *
 **/
void LiquidCrystal_I2LCD::home(void)
{
    _command(CURSOR_HOME, 0x00);
    column = 0;
    row = 0;
}

/**
 * @brief Clear display by filling DDRAM with 0x20
 * characters (spaces) and set column and row
 * to 0,0
 *
 **/
void LiquidCrystal_I2LCD::clear(void)
{
    _command(CLEAR_DISPLAY, 0x00);
    column = 0;
    row = 0;
}

/**
 * @brief Turns off blink function
 *
 **/
void LiquidCrystal_I2LCD::noBlink(void)
{
    _command(DISPLAY_ONOFF, commands[DISPLAY_ONOFF] & (~DOO_B));
}

/**
 * @brief Turns on blink function
 *
 **/
void LiquidCrystal_I2LCD::blink(void)
{
    _command(DISPLAY_ONOFF, commands[DISPLAY_ONOFF] | DOO_B);
}

/**
 * @brief Turns off cursor funtion
 *
 **/
void LiquidCrystal_I2LCD::noCursor(void)
{
    _command(DISPLAY_ONOFF, commands[DISPLAY_ONOFF] & (~DOO_C));
}

/**
 * @brief Turns on cursor funtion
 *
 **/
void LiquidCrystal_I2LCD::cursor(void)
{
    _command(DISPLAY_ONOFF, commands[DISPLAY_ONOFF] | DOO_C);
}

/**
 * @brief Turns off LCD display
 * All data in CG/DD ram retains
 *
 **/
void LiquidCrystal_I2LCD::noDisplay(void)
{
    _command(DISPLAY_ONOFF, (commands[DISPLAY_ONOFF] & (~DOO_D)));
}


/**
 * @brief Turns on LCD display
 * All data in CG/DD ram retains
 *
 **/
void LiquidCrystal_I2LCD::display(void)
{
    _command(DISPLAY_ONOFF, commands[DISPLAY_ONOFF] | DOO_D);
}

/**
 * @brief Set cursor postition to given column and row
 *
 *
 * @param column
 * @param row
 **/
void LiquidCrystal_I2LCD::setCursor(uint8_t pcol, uint8_t prow)
{
    _command(SET_DDRAM_ADDRESS, row_offsets[prow] + pcol);
    column = pcol;
    row = prow;
}

/**
 * @brief Left justify text from the cursor
 *
 **/
void LiquidCrystal_I2LCD::noAutoscroll(void)
{
  _command(ENTRY_MODE_SET, (commands[ENTRY_MODE_SET] & (~EMS_S)));
}

/**
 * @brief Right justify text from the cursor
 *
 **/
void LiquidCrystal_I2LCD::autoscroll(void)
{
  _command(ENTRY_MODE_SET, commands[ENTRY_MODE_SET] | EMS_S);
}

/**
 * @brief Scroll the display to the left
 *
 **/
void LiquidCrystal_I2LCD::scrollDisplayLeft(void)
{
  _command(CURSOR_DISPLAY_SHIFT, commands[CURSOR_DISPLAY_SHIFT] & (~CDS_RL) | CDS_SC);
}

/**
 * @brief Scroll the display to the right
 *
 **/
void LiquidCrystal_I2LCD::scrollDisplayRight(void)
{
  _command(CURSOR_DISPLAY_SHIFT, commands[CURSOR_DISPLAY_SHIFT] | CDS_RL | CDS_SC);
}

/**
 * @brief Text if flowing to the right
 *
 **/
void LiquidCrystal_I2LCD::leftToRight(void)
{
    _command(ENTRY_MODE_SET, commands[ENTRY_MODE_SET] | EMS_ID);
}

/**
 * @brief Text if flowing to the left
 *
 **/
void LiquidCrystal_I2LCD::rightToLeft(void)
{
    _command(ENTRY_MODE_SET, commands[ENTRY_MODE_SET] & (~EMS_ID));
}

/**
 * @brief Create graphical character from given buffer.
 * @param character number to be defined
 * @param buffer with bitmap
 * @param count number of bytes to be transfered (default 8).
 *
 */
void LiquidCrystal_I2LCD::createChar(uint8_t location, uint8_t charmap[], uint8_t count)
{
  location &= 0x7; // we only have 8 locations 0-7
  count &= 0x3f;

  _command(SET_CGRAM_ADDRESS, (location << 3));
  _writeblock((const char*)charmap, count);
  setCursor(column, row); //We should get back to DD RAM addressing mode
}

/**
 * @brief Read content of given graphical character into buffer.
 * @param character number
 * @param buffer with at least count bytes
 * @param counter number of bytes to transfer
 */
void LiquidCrystal_I2LCD::readChar(uint8_t location, uint8_t *charmap, uint8_t count)
{
  location &= 0x7; // we only have 8 locations 0-7
  count &= 0x3f;

  _command(SET_CGRAM_ADDRESS, (location << 3));
  _readblock((char *)charmap, count);
  setCursor(column, row); //We should get back to DD RAM addressing mode
}

/**
 * @brief Set backlight brightness. Value ranges between 0 and 63
 * 0 - minimum brightness, 63 - maximum brightness.
 *
 **/
void LiquidCrystal_I2LCD::setBacklight(uint8_t value)
{
    _set(BACKLIGHT_CS, value);
}

/**
 * @brief Set display contrast. Value ranges between 0 and 63
 * 0 - the display is darkest , 63 - the display is lightest.
 *
 **/
void LiquidCrystal_I2LCD::setContrast(uint8_t value)
{
    _set(CONTRAST_CS, 0x3f - potTransTable[value]);
}

int LiquidCrystal_I2LCD::init()
{
   int status = 0;
   uint8_t fn;

   // initialize the backpack IO expander
   // and display functions.
   // ------------------------------------------------------------------------
   if ( PCA9535::begin ( address ) == 1 )
   {
        setDirection(CPORT, IRS);  //Only IRS is set to input on control port
        setDirection(DPORT, 0x00); //Data port is set to output by default
        setOutput(CPORT, (PWR | UD | BACKLIGHT_CS | CONTRAST_CS));
        control = getOutput(CPORT);

        brightness = 0x00;
        contrast = 0x00;

        _usleep(4500);
        fn = FS_DL | (rows > 1 ? FS_N : 0);
        _command(FUNCTION_SET, fn);
        _usleep(5000);
        _command(FUNCTION_SET, fn);
        _usleep(4500);
        _command(FUNCTION_SET, fn);
        _usleep(4500);
        _command(DISPLAY_ONOFF, 0x00);
        _usleep(6000);
        _command(CLEAR_DISPLAY, 0x00);
        _usleep(30000);
        _command(ENTRY_MODE_SET, EMS_ID);
        _usleep(6000);
        _command(DISPLAY_ONOFF, DOO_D);
        _usleep(6000);
        waitflag = true;

        for(int i=0; i<64; i++)
        {
            _dec(BACKLIGHT_CS);
            _dec(CONTRAST_CS);
        }

        Serial.println("Init done.");
   }
   return ( status );
}

/**
 * @brief write byte to current DDRAM address
 * @param byte to write
 * @return 1 as one byte will be written
  */
size_t LiquidCrystal_I2LCD::write(uint8_t value)
{
    //while(waitflag && (_status() & BUSY_FLAG)) {};
    _control(RS, 1);
    _control(RW, 0);
    _control(EN, 1);
    _usleep(1);
    setOutput(DPORT, value);
    _usleep(1);
    _control(EN, 0);
    _control(RS | RW | EN, 0);
  return 1; // assume sucess
}

/**
 * @brief Read a character from the display at current cursor position.
 * @return a character
 **/
char LiquidCrystal_I2LCD::read(void)
{
    char retVal;

    //setCursor(column, row); //Actually it sends SET_DDRAM_ADDRESS command to
                            //the display, so next read can fetch value from
                            //this address
    _control(RS | RW, 1);
    setDirection(DPORT, 0xFF);
    _usleep(1);
    _control(EN, 1);
    retVal = (char)getPort(DPORT);
    _usleep(1);
    _control(EN, 0);
    _control(RS | RW, 0);
    setDirection(DPORT, 0x00);
    //setCursor(column, row);
    return retVal;
}

/**
 * @brief Read whole row from the display
 * @return address of array containing row read from the display.
 *         If you would like to manipulate those bytes, you should copy them
 *         to the buffer not defined as const.
 **/
const char *LiquidCrystal_I2LCD::read(uint8_t row)
{
    int i;
    setCursor(0, row); //Actually it sends SET_DDRAM_ADDRESS command to
                            //the display, so next read can fetch value from
                            //this address
    _control(RS | RW, 1);
    setDirection(DPORT, 0xFF);
    for(i=0; i<columns; i++)
    {
        _usleep(1);
        _control(EN, 1);
        buffer[i] =  (char)getPort(DPORT);
        _usleep(1);
        _control(EN, 0);
    }
    buffer[i] = 0;
    _control(RS | RW, 0);
    setDirection(DPORT, 0x00);
    setCursor(column, row);
    return buffer;
}

/**
 * @brief Method for write CG RAM with content of a buffer.
 *        Differs from createChar in a way it treats CG RAM as one block
 *        of ram to write and first argument to this method is address of first
 *        CG RAM byte to be written. CG RAM in HD4470 has 64 bytes (8 bytes * 8
 *        characters), but it is linear region in a sense, there is no boundary between
 *        each character, so we can treat it as one block of RAM.
 * @param first adddress of CG RAM
 * @param buffer to transfer
 * @param count number of bytes to transfer.
 */
void LiquidCrystal_I2LCD::writeCGRAM(uint8_t address, const char *buffer, uint8_t count)
{
  _command(SET_CGRAM_ADDRESS, address & 0x3f);
  _writeblock((const char*)buffer, count);
  setCursor(column, row); //We should get back to DD RAM addressing mode
}

/**
 * @brief Method opposite to writeCGRAM used to read data from CG RAM.
 * @param first byte addresses of CG RAM to read from
 * @param buffer to write content of CG RAM
 * @param counter
 */
void LiquidCrystal_I2LCD::readCGRAM(uint8_t address, char *buffer, uint8_t count)
{
  _command(SET_CGRAM_ADDRESS, address & 0x3f);
  _readblock((char*)buffer, count);
  setCursor(column, row); //We should get back to DD RAM addressing mode
}

/**
 * @brief Changes CPORT outputs to control and LCD
 * expects values to set/unset and boolean
 * value controling if value should be set
 * or unset.
 * This method is private
 * @param flags
 * @param value
 **/
void LiquidCrystal_I2LCD::_control(uint8_t flags, bool value)
{
    control = value ? (control | flags) : (control & (~flags));
    setOutput(CPORT, control);
}

/**
 * @brief Sends commands to an LCD.
 * Inputs are command (bit number of command)
 * and value to send with command
 * This method is private
 *
 * @param command
 * @param value
 **/
void LiquidCrystal_I2LCD::_command(t_Command command, uint8_t value)
{
    while(waitflag && (_status() & BUSY_FLAG)) {};
    value |= (1 << (uint8_t) command);
    _usleep(1);
    _control(RS|RW, 0);
    _usleep(1);
    _control(EN, 1);
    _usleep(1);
    setOutput(DPORT, value);
    _usleep(2);
    _control(EN, 0);
    commands[(uint8_t)command] = value;
}

/**
 * @brief Returns internal status of an LCD.
 * If last operation sent to an LCD was setting
 * DD/CGRAM address, it will also return current
 * address of DD/CGRAM in first 7 bits of status.
 * Eighth bit is LCD in operation status.
 * 1 - means LCD is busy
 * 0 - LCD can execute another operation
 * This method is private
 * @return status flag
 **/
uint8_t LiquidCrystal_I2LCD::_status(void)
{
    uint8_t ret;

    _control(RS, 0);
    _control(RW, 1);
    setDirection(DPORT, 0xFF);
    _control(EN, 1);
    ret = getPort(DPORT);
    _usleep(1);
    _control(RS | RW | EN, 0);
    setDirection(DPORT, 0x00);
    return ret;
}

/**
 * @brief Controls UD line of potentiometers. UD line is common for both
 *        potentiometers of the module. Which pot is currently used depends
 *        on selected CS line.
 * @param boolean value 0 - set line to LOW, 1 - set line to HIGH state
 **/
void LiquidCrystal_I2LCD::_ud(bool v)
{
    _control(UD, v);
}

/**
 * @brief Controls CS line of potentiometer.
 * @param value of bit representing CS line of pot.
 * @param boolean value 0 - set line to LOW, 1 - set line to HIGH state
 **/
void LiquidCrystal_I2LCD::_cs(uint8_t cspin, bool v)
{
    _control(cspin, v);
}

/**
 * @brief Decrement pot value by 1.
 *        0 is minimum value which wiper can reach.
 * @param value of bit representing CS line of pot.
 **/
void LiquidCrystal_I2LCD::_dec(uint8_t v)
{
    if (v == BACKLIGHT_CS)
        brightness -= brightness > 0 ? 1 : 0;
    else if (v == CONTRAST_CS)
        contrast -= contrast > 0 ? 1 : 0;
    else
        return;

    _ud(0);
    _usleep(1);
    _cs(v, 0);
    _ud(1);
    _usleep(1);
    _ud(0);
    _usleep(5);
    _cs(v, 1);
    _usleep(5);
}

/**
 * @brief Increment pot value by 1.
 *        63 is maximum value of wiper.
 * @param value of bit representing CS line of pot.
 **/
void LiquidCrystal_I2LCD::_inc(uint8_t v)
{
    if (v == BACKLIGHT_CS)
        brightness += brightness < 63 ? 1 : 0;
    else if (v == CONTRAST_CS)
        contrast += contrast < 63 ? 1 : 0;
    else
        return;

    _ud(1);
    _usleep(1);
    _cs(v, 0);
    _ud(0);
    _usleep(1);
    _ud(1);
    _usleep(5);
    _cs(v, 1);
    _usleep(5);
}

/**
 * @brief Set wiper of the pot to given value.
 *        We can't read current wiper position from the pots, so we remembering
 *        current wiper position in 'contrast' and 'brighness' variables.
 *        This function calculate difference between current wiper value and
 *        given, and depending on sign of this difference it will incerement
 *        or decrement wiper of given pot, until it reaches 'value'
 * @param bit representing CS line of pot.
 * @param value pot value
 **/
void LiquidCrystal_I2LCD::_set(uint8_t v, uint8_t value)
{
    uint8_t current, i;
    char diff;

    if (v == BACKLIGHT_CS)
        current = brightness;
    else if (v == CONTRAST_CS)
        current = contrast;
    else
        return;

    value &= 0x3f;

    if (value > current)
    {
	diff = value - current;
	for (i=0; i < diff; i++)
            _inc(v);
    } else if (value < current)
    {
	diff = current - value;
	for(i=0; i < diff; i++)
            _dec(v);
    }
}

/**
 * @brief Writes array of bytes to the internal RAM of the display.
 * RAM type depends on previous command issued. For CG RAM you should call
 * SET_CGRAM_ADDRESS, for DD RAM SET_DDRAM_ADDRESS should be called in prior
 * Counter of RAM address will be automatically incremented after each write.
 *
 * @param array of bytes to be written
 * @param lenght of the array
 */
void LiquidCrystal_I2LCD::_writeblock(const char *block, uint8_t len)
{
    uint8_t i;

    _control(RS, 1);
    _control(RW, 0);
    for(i=0; i<len; i++)
    {
	_control(EN, 1);
	_usleep(1);
	setOutput(DPORT, block[i]);
	_control(EN, 0);
	_usleep(1);
    }
    _control(RS | RW | EN, 0);
}

/**
 * @brief Reads array of bytes form the internal RAM of the display.
 * RAM type depends on previously issued command. If CG RAM was issued
 * array will be filled with bytes from CG RAM.
 * Counter of RAM address will be automatically incremented after each read.
 *
 * @param array of bytes to be read to
 * @param lenght of the array
 */
void LiquidCrystal_I2LCD::_readblock(char *block, uint8_t len)
{
    uint8_t i;

    _control(RS, 1);
    _control(RW, 1);
    setDirection(DPORT, 0xFF);
    for(i=0; i<len; i++)
    {
	_control(EN, 1);
	block[i] = (char)getPort(DPORT);
	_usleep(1);
	_control(EN, 0);
	_usleep(1);
    }
    setDirection(DPORT, 0x00);
    _control(RS | RW | EN, 0);
}

/**
 * @brief Helper function to set offset of rows addresses depending on current
 * construction of the display.
 **/
void LiquidCrystal_I2LCD::_offset(t_LCDType type)
{
    t_LCDType t = type;
    if(type == D16x1)
        t = D8x2;

    uint16_t tmp = _deinterleave((uint16_t) t);
    *((uint32_t*)row_offsets) = 0x00;

    columns = tmp >> 8;
    rows = (tmp & 0x00ff) ? (tmp & 0x00ff) : 1; //D16x0 means D16x1 with linear addressing

    if (type == D6x1 || type == D8x1 || type == D16x0)
        return;

    row_offsets[1] = 0x40;
    if (type == D8x2 || type == D16x1 || type == D16x2 || type == D20x2 || type == D24x2 || type == D40x2)
        return;


    row_offsets[2] = 0x10;
    row_offsets[3] = 0x50;

    if (type == D20x4)
    {
        row_offsets[2] |= 4;
        row_offsets[3] |= 4;
    }
}

/**
 * @brief Helper function to space argument bits apart, so we can interleve
 *        them with second argument.
 * @param argument to interleve
 */
uint16_t LiquidCrystal_I2LCD::_partbyte(uint16_t n)
{
    n &= 0x00ff;
    n = (n | (n << 8)) & 0x00ff00ff;
    n = (n | (n << 4)) & 0x0f0f0f0f;
    n = (n | (n << 2)) & 0x33333333;
    n = (n | (n << 1)) & 0x55555555;
    return n;
}

/**
 * @brief Reverse _partbyte() method, to get two arguments back in
 *        deinterleave operation.
 * @param argument to deinterleave
 */
uint16_t LiquidCrystal_I2LCD::_unpartbyte(uint16_t n)
{
    n &= 0x5555;
    n = (n ^ (n >> 1)) & 0x33333333;
    n = (n ^ (n >> 2)) & 0x0f0f0f0f;
    n = (n ^ (n >> 4)) & 0x00ff00ff;
    n = (n ^ (n >> 8)) & 0x0000ffff;
    return n;
}

/**
 * @brief interleves bits from width and height of the display
 *        and for respresenting them as one integer value
 * @param width of the display
 * @param height of the display
 * @return interleved values of width and height
 */
uint16_t LiquidCrystal_I2LCD::_interleave(uint8_t width, uint8_t height)
{
    return _partbyte(width) | (_partbyte(height) << 1);
}

/**
 * @brief opposite function to the _interleave(). Returns 16 bit value
 *        with most significant byte set to width of the display
 *        and last significant byte set to height of the display.
 * @param value of interleved width and height
 * @return two bytes in 16 bit word.
 */
uint16_t LiquidCrystal_I2LCD::_deinterleave(uint16_t n)
{
    return (_unpartbyte(n) << 8) | _unpartbyte(n >> 1);
}

/**
 * @brief function for waiting in us range.
 */
void LiquidCrystal_I2LCD::_usleep(uint32_t usdelay)
{
    delayMicroseconds(usdelay);
}

