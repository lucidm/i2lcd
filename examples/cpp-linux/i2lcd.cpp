#include <string.h>
#include <unistd.h>
#include <exception>
#include <i2lcd.h>

using namespace i2lcd;

const uint8_t potTransTable[64] = {0,10,16,21,24,27,29,31,33,35,36,37,39,40,41,42,43,44,44,45,46,47,47,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,55,56,56,57,57,57,58,58,58,59,59,59,60,60,60,60,61,61,61,62,62,62,62,63,63,63};

/**
 * @brief Helper function to space argument bits apart, so we can interleve
 *        them with second argument.
 * @param argument to interleve
 */
static uint16_t _partbyte(uint16_t n)
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
static uint16_t _unpartbyte(uint16_t n)
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
static uint16_t _interleave(uint8_t width, uint8_t height)
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
static uint16_t _deinterleave(uint16_t n)
{
    return (_unpartbyte(n) << 8) | _unpartbyte(n >> 1);
}



/**
 * @brief LcdType class constructor.
 * This class maintains an LCD configuration
 * rows addresses. Rows and Columns counters etc
 *
 **/
LcdType::LcdType(t_LCDType type) : lcdtype(type)
{
    t_LCDType t = type;
    if(type == D16x1)
        t = D8x2;

    uint16_t tmp = _deinterleave((uint16_t) t);
    *((uint32_t*)rowaddr) = 0x00;

    columns = tmp >> 8;
    rows = (tmp & 0x00ff) ? (tmp & 0x00ff) : 1; //D16x0 means D16x1 with linear addressing
    lines = rows > 1;

    if (type == D6x1 || type == D8x1 || type == D16x0)
        return;

    rowaddr[1] = 0x40;
    if (type == D8x2 || type == D16x1 || type == D16x2 || type == D20x2 || type == D24x2 || type == D40x2)
        return;


    rowaddr[2] = 0x10;
    rowaddr[3] = 0x50;

    if (type == D12x4)
    {
        rowaddr[2] -= 4;
        rowaddr[3] -= 4;
    }

    if (type == D20x4)
    {
        rowaddr[2] |= 4;
        rowaddr[3] |= 4;
    }
}

/**
 * @brief [] operator for LcdType class
 * returns address of row given as index or
 * throws RowOutOfRange exception if given row is
 * is beyond actual range of an LCD.
 * @param row
 * @return address of a row in DDRAM
 **/
uint8_t LcdType::operator[](uint8_t row) const
{
    if (row > rows) throw RowOutOfRange();
    return rowaddr[row];
}

/**
 * @brief Method returns address of byte in DDRAM
 * by given column and row number.
 * throws Row/ColumnOutOfRange exception if
 * given row/column values are beyond actual
 * range of an LCD
 * @param column
 * @param row
 * @return address of a row in DDRAM
 **/
uint8_t LcdType::ddAddress(uint8_t column, uint8_t row) const
{
    if (row > rows) throw RowOutOfRange();
    if (column > columns) throw ColumnOutOfRange();
    return rowaddr[row] + column;
}

/**
 * @brief Method returns address of CGRAM by given
 * character number and row in character.
 * It can throw one of two excetpions:
 * CharacterOutOfRange - when given character has
 *                       number greater than 7
 * or RowOutOfNumber - when given row exceeds
 *                     maximum height of character
 *                     (usually 7)
 * @param character
 * @param row
 * @return address of a byte in CGRAM
 **/
uint8_t LcdType::cgAddress(uint8_t character, uint8_t row) const
{
    if (row > 7) throw RowOutOfRange();
    if (character > 7)  throw CharacterOutOfRange();
    return (8 * character) + row;
}

void I2Lcd::_init(void)
{
    setDirection(CPORT, IRS);
    setDirection(DPORT, 0x00);
    setOutput(CPORT, (UD | BACKLIGHT_CS | CONTRAST_CS));

    bpot = new Potentiometer(*this, BACKLIGHT_CS, UD);
    cpot = new Potentiometer(*this, CONTRAST_CS, UD);

    control = getOutput(CPORT);
    setOutput(CPORT, control);

    row = 0;
    column = 0;
    memset(commands, 0, 8);
}

/**
 * @brief I2Lcd class constructor.
 * Requires bus number, address of the i2lcd module
 * and type of an LCD.
 *
 * @param bus number
 * @param address I2C address of module
 * @param type type of an LCD connected to bus
 **/
I2Lcd::I2Lcd(uint8_t bus, uint8_t address, t_LCDType type) : PCA9535(bus, address), lcdtype(LcdType(type)), control(0), waitflag(0)
{
    _init();
}

/**
 * @brief Nother I2Lcd class constructor.
 * Requires bus number, address of the i2lcd module
 * columns and rows configuration.
 *
 * @param bus number
 * @param address I2C address of module
 * @param number of columns
 * @param number of rows the display has
 **/
I2Lcd::I2Lcd(uint8_t bus, uint8_t address, uint8_t columns, uint8_t rows) : PCA9535(bus, address),
                                                                            control(0), waitflag(0)
{
    lcdtype = LcdType((t_LCDType)_interleave(columns, rows));
    _init();
}

/**
 * @brief Destructor of I2Lcd class.
 * Turns pots all the way down to 0, change
 * all outputs of i2lcd module to input
 * and switches power off.
 **/
I2Lcd::~I2Lcd()
{
    bpot->set(0x00);
    cpot->set(0x3f);
    setOutput(CPORT, ~(PWR));
    if (cpot) delete cpot;
    if (bpot) delete bpot;
    setDirection(CPORT, 0xFF);
    setDirection(DPORT, 0xFF);
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
void I2Lcd::_control(uint8_t flags, bool value)
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
void I2Lcd::_command(t_Command command, uint8_t value)
{
    while(waitflag && (_status() & BUSY_FLAG)) {};
    value |= (1 << (uint8_t) command);
    usleep(1);
    _control(RS|RW, 0);
    usleep(1);
    _control(EN, 1);
    usleep(1);
    setOutput(DPORT, value);
    usleep(2);
    _control(EN, 0);
    commands[(uint8_t)command] = value;
}

/**
 * @brief Writes block data to LCD.
 * Some commands provide continuos transfers.
 * Setting DDRAM address or CGRAM address will
 * expect next byte to be transferred to subsequent
 * address without setting new DD/CGRAM address.
 * LCD will internally increment those addresses
 * after each byte transferred.
 * Inputs are array of bytes to transfer and
 * number of bytes should be transferred.
 * This method is private
 *
 * @param block
 * @param len
 **/
void I2Lcd::_writeblock(const char *block, uint8_t len)
{
    uint8_t i;

    _control(RS, 1);
    _control(RW, 0);
    for(i=0; i<len; i++)
    {
	_control(EN, 1);
	usleep(1);
	setOutput(DPORT, block[i]);
	_control(EN, 0);
	usleep(1);
    }
    _control(RS | RW | EN, 0);
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
uint8_t I2Lcd::_status(void)
{
    uint8_t ret;

    _control(RS, 0);
    _control(RW, 1);
    setDirection(DPORT, 0xFF);
    _control(EN, 1);
    ret = getPort(DPORT);
    usleep(1);
    _control(RS | RW | EN, 0);
    setDirection(DPORT, 0x00);
    return ret;
}

/**
 * @brief Set brightness of backlight
 * Allowed values are from 0 (darkest) to
 * 0x3f (brightest)
 *
 * @param value
 **/
void I2Lcd::setBacklight(uint8_t value) { bpot->set(value); };

/**
 * @brief Set contrast of and LCD.
 * However alowed values are from 0 to 0x3f,
 * reasonable range depends on counterpart LCD.
 * It also depends on environment temperature
 * of an LCD. Some LCDs even have reversed polarity
 * of contrast cirquits, so the range in this case
 * should also be reversed.
 * You should try yourself with your LCD module.
 *
 * @param value
 **/
void I2Lcd::setContrast(uint8_t value) { cpot->set(0x3f - potTransTable[value]); };

/**
 * @brief Switch power of an LCD on or off.
 * If state of power is changing from off to on
 * init() method will be called also.
 *
 * @param value true to switch power on,
 *              otherwise poweroff
 **/
void I2Lcd::power(bool value)
{
        _control(PWR, value);
	if (control & PWR)
	    init();
	else
	    waitflag = 0;
}

/**
 * @brief Init method is called whenever LCD is switched
 * on. Sets interface configuration, clears display
 * and switches display on.
 *
 *
 **/
void I2Lcd::init(void)
{
    usleep(4500);
    uint8_t fn = FS_DL | (lcdtype.getLine() ? FS_N : 0);
    _command(FUNCTION_SET, fn);
    usleep(5000);
    _command(FUNCTION_SET, fn);
    usleep(4500);
    _command(FUNCTION_SET, fn);
    usleep(4500);
    _command(DISPLAY_ONOFF, 0x00);
    usleep(6000);
    _command(CLEAR_DISPLAY, 0x00);
    usleep(30000);
    _command(ENTRY_MODE_SET, EMS_ID);
    usleep(6000);
    _command(DISPLAY_ONOFF, DOO_D);
    usleep(6000);
    waitflag = 1;
}


/**
 * @brief Set cursor postition to given column and row
 *
 *
 * @param column
 * @param row
 **/
void I2Lcd::setCursor(uint8_t pcol, uint8_t prow)
{
    _command(SET_DDRAM_ADDRESS, lcdtype.ddAddress(pcol, prow));
    column = pcol;
    row = prow;
}

/**
 * @brief Call internal LCD command "return home".
 * Cursor will appear in first column and row
 * Class row and column also will be set to
 * first row and column.
 *
 **/
void I2Lcd::home(void)
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
void I2Lcd::clear(void)
{
    _command(CLEAR_DISPLAY, 0x00);
    column = 0;
    row = 0;
}

/**
 * @brief Set bitmap for given character
 * bitmap should contain 8 bytes defining bitmap
 *
 * @param character number
 * @param bitmap address
 **/
void I2Lcd::setGC(uint8_t character, const char *bitmap)
{
    _command(SET_CGRAM_ADDRESS, lcdtype.cgAddress(character, 0));
    _writeblock(bitmap, 8);
}

/**
 * @brief Turns on or off blink function
 *
 * @param value true or false
 **/
void I2Lcd::blink(bool value)
{
    _command(DISPLAY_ONOFF, value ? (commands[DISPLAY_ONOFF] | DOO_B) : (commands[DISPLAY_ONOFF] & (~DOO_B)));
}

/**
 * @brief Turns on or off cursor funtion
 *
 * @param value true or false
 **/
void I2Lcd::cursor(bool value)
{
    _command(DISPLAY_ONOFF, value ? (commands[DISPLAY_ONOFF] | DOO_C) : (commands[DISPLAY_ONOFF] & (~DOO_C)));
}

/**
 * @brief Turns on or off LCD display
 * All data in CG/DD ram retains
 *
 * @param value true or false
 **/
void I2Lcd::display(bool value)
{
    _command(DISPLAY_ONOFF, value ? (commands[DISPLAY_ONOFF] | DOO_D) : (commands[DISPLAY_ONOFF] & (~DOO_D)));
}

/**
 * @brief Return content of given row as string
 * Function reads DDRAM content of an LCD
 *
 * @param row number
 * @return string
 **/
string I2Lcd::getRow(uint8_t row)
{
    uint8_t i;
    string s;

    _command(SET_DDRAM_ADDRESS, lcdtype.ddAddress(0, row));
    _control(RS | RW, 1);
    setDirection(DPORT, 0xFF);
    for(i=0; i<lcdtype.getColumns(); i++)
    {
	usleep(1);
	_control(EN, 1);
	s += (char)getPort(DPORT);
	usleep(1);
	_control(EN, 0);
    }
    _control(RS | RW, 0);
    setDirection(DPORT, 0x00);
    _command(SET_DDRAM_ADDRESS, lcdtype.ddAddress(column, row));
    return s;
}

/**
 * @brief Print string at current position.
 * If string is longer than space available at
 * current row, it will continue to print rest
 * at new row. At last row it will reset row to
 * 0 again, effectively wrapping string around
 * an LCD.
 *
 * @param string value
 **/
void I2Lcd::print(string value)
{
    std::string::iterator i;
    char c;

    uint8_t cl, rw, idc = 0;

    cl = column;
    rw = row;

    for (i=value.begin(); i!=value.end(); i++)
    {
	c = *i;
	if (c == '\n')
	{
	    rw = (rw + 1) % rows();
	    cl = 0;
	    continue;
	} else
	{
	    _command(SET_DDRAM_ADDRESS, lcdtype.ddAddress(cl, rw));
	    _writeblock(&c, 1);
	    cl++;
	    if(cl == columns())
	    {
		if (idc == 0) idc = 1;
		cl = 0;
	    }
	}
	if (idc)
	{
	    rw = (rw + 1) % rows();
	    idc = 0;
	}
    }
    row = rw;
    column = cl < columns() ? cl : columns() - 1;
    _command(SET_DDRAM_ADDRESS, lcdtype.ddAddress(column, row));
}

/**
 * @brief Return current row content. Subsequent call
 * of this function will return next row, until
 * the last row of an LCD is reached. Next call
 * will return content of first row again.
 *
 * @return string
 **/
string I2Lcd::getRow(void)
{
    string s = getRow(row);
    row = (row + 1) % lcdtype.getRows();
    return s;
}

/**
 * @brief Operator behaves same as getRow(row) method.
 * You can use it as equvalent of mentioned method
 *
 * @param integer as index of the operator
 * @return string
 **/
string I2Lcd::operator[](uint8_t row)
{
    return getRow(row);
}


void I2Lcd::_dump(void)
{
    uint8_t i;
    string s;

    s += "+";
    for(i=0; i < lcdtype.getColumns(); i++) s += "-";
    s+="+\n";

    for(i=0; i < lcdtype.getRows(); i++)
    {
	s += "|" + getRow(i) + "|\n";
    }
    s += "+";

    for(i=0; i < lcdtype.getColumns(); i++) s += "-";
    s+="+\n";

    std::cout << s;
}


using namespace std;

/**
 * @brief Dumps all content of an LCD to ostream object
 *
 *
 * @param ostream object reference
 * @param I2Lcd object reference
 * @return ostream object reference
 **/
std::ostream &operator<<(std::ostream &os, I2Lcd &lcd)
{
    uint8_t i;
    for (i=0; i < lcd.rows(); i++)
	os << lcd.getRow(i) << endl;
    return os;
}

