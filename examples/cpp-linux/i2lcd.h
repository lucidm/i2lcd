#ifndef __I2LCD_H__
#define __i2LCD_H__

#include <string>
#include <iostream>

#include <pca9535.h>
#include <pots.h>




using namespace i2lcd;

#define	EN	1
#define RW	(1 << 1)
#define RS	(1 << 2)
#define IRS	(1 << 3)
#define PWR	(1 << 4)

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

#define EMS_S	1
#define EMS_ID	(1 << 1)

#define DOO_B	1
#define DOO_C	(1 << 1)
#define DOO_D	(1 << 2)

#define CDS_RL	(1 << 2)
#define CDS_SC	(1 << 3)

#define FB_F	(1 << 2)
#define FS_N	(1 << 3)
#define FS_DL	(1 << 4)

#define BUSY_FLAG	(1 << 7)

#define POWERON	1
#define POWEROFF 0

namespace i2lcd
{

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

/**
 * @class ColumnOutOfRange
 *
 * @ingroup i2lcd
 *
 * @brief Column out of range exception
 *
 * Class will be thrown as exception if given number of column is
 * out of range. LCD has less columns than given
 *
 *
 *
 */
class ColumnOutOfRange : public exception
{
    virtual const char *what() const throw() { return "Column number out of range"; };
};

/**
 * @class RowOutOfRange
 *
 * @ingroup i2lcd
 *
 * @brief Row out of range exception
 *
 * Class will be thrown as exception if given number of row is
 * out of range. LCD has less rows than given
 *
 *
 */
class RowOutOfRange : public exception
{
    const char *what() const throw() { return "Row number out of range"; };
};

/**
 * @class CharacterOutOfRange
 *
 * @ingroup i2lcd
 *
 * @brief Character number out of range exception
 *
 * Class will be thrown as exception if given graphicall character number is
 * out of range. LCD usually has 8 characters numbered from 0 to 7
 *
 *
 */
class CharacterOutOfRange : public exception
{
    const char *what() const throw() { return "Character number out of range"; };
};

/**
 * @class LcdType
 *
 * @ingroup i2lcd
 *
 * @brief Contains methods and values characteristic for given type of an LCD
 *
 * Helper class for I2Lcd class.
 * Knows different types LCDs memory organization, number of rows and columns.
 *
 */
class LcdType
{
    private:
	t_LCDType lcdtype;
	uint8_t columns;
	uint8_t rows;
	uint8_t lines;
	uint8_t rowaddr[4];

    public:
        LcdType() {};
	LcdType(t_LCDType);
	uint8_t getRows() const { return rows; };
	uint8_t getColumns() const { return columns; };

	uint8_t getRowAddress(uint8_t number) const;
	uint8_t getLine() const { return lines; };
	uint8_t ddAddress(uint8_t column, uint8_t row) const;
	uint8_t cgAddress(uint8_t character, uint8_t row) const;
	uint8_t operator[](uint8_t row) const;
};

/**
 * @class I2Lcd
 *
 * @ingroup i2lcd
 *
 * @brief Main module interface class.
 *
 * Class implements metohds for accessing LCD connected through I2LCD module
 *
 *
 */
class I2Lcd : public PCA9535
{
    private:
	LcdType lcdtype;
	Potentiometer *cpot;
	Potentiometer *bpot;
	uint8_t control;
	uint8_t column;
	uint8_t row;
	bool waitflag;
	uint8_t commands[8];

	void _control(uint8_t flags, bool value);
	void _command(t_Command command, uint8_t value);
	uint8_t _status(void);
	void _writeblock(const char *block, uint8_t len);
        void _readblock(const char *block, uint8_t len);
        void _init(void);


    public:
	I2Lcd(uint8_t bus, uint8_t address, t_LCDType type);
        I2Lcd(uint8_t bus, uint8_t address, uint8_t columns, uint8_t rows);
	~I2Lcd();
	uint8_t rows(void) {return lcdtype.getRows(); };
	uint8_t columns(void) {return lcdtype.getColumns(); };
	void setBacklight(uint8_t value);
	void setContrast(uint8_t value);
	void setCursor(uint8_t pcol, uint8_t prow);
	void setGC(uint8_t character, const char *bitmap);
	string getRow(uint8_t row);
	string getRow(void);
	void power(bool value);
	void init(void);
	void home(void);
	void clear(void);
	void blink(bool value);
	void cursor(bool value);
	void display(bool value);
	void print(string value);
	string operator[](uint8_t row);

	void _dump(void);
};

};


#endif
