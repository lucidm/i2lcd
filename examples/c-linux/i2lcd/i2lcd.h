#ifndef __I2LCD_H__
#define __I2LCD_H__
#include <stdint.h>
#include <linux/i2c-dev.h>

#include <pca9535.h>
#include <pots.h>


#define CPORT 0 /**< Control port of module all control lines are connected to*/
#define DPORT 1 /**< Data port of the module, data lines of LCD are connected to*/

#define EN 1 /**< Bit value where EN line is connected*/
#define RW  (1 << 1) /**< RW line bit value*/
#define RS  (1 << 2) /**< RS line bit value*/
#define IRS (1 << 3) /**< INT Line, RW/RS/EN and D7 lines are glued to this line*/
#define PWR (1 << 4) /**< Power line bit number*/

/**
 * @brief Commands type definitions.
 * As each command is defined by different bit number, this type contains
 * only bit numbers.
 */
typedef enum command {
	CLEAR_DISPLAY=0,
	CURSOR_HOME,
	ENTRY_MODE_SET,
	DISPLAY_ONOFF,
	CURSOR_DISPLAY_SHIFT,
	FUNCTION_SET,
	SET_CGRAM_ADDRESS,
	SET_DDRAM_ADDRESS,
} t_Command;


//ENTRY_MODE_SET arguments
#define EMS_S 	1    /**< ENTRY MODE SET: Shift display bit*/
#define EMS_ID 	(1 << 1) /**< ENTRy MODE SET: Increment/Decrement address */

//DISPLAY_ONOFF arguments
#define DOO_B	1     /**< DISPLAY ON/OFF: Blinking */
#define DOO_C	(1 << 1) /**< DISPLAY ON/OFF: Cursor */
#define DOO_D	(1 << 2) /**< DISPLAY ON/OFF: Display */

//CURSOR_DISPLAY_SHIFT arguments
#define CDS_RL (1 << 2) /**< CURSOR DISPLAY SHIFT: left/right direction */
#define CDS_SC (1 << 3) /**< CURSOR DISPLAY SHIFT: shift display w/o DDRAM cont. change*/

//FUNCTION_SET arguments
#define FB_F	(1 << 2) /**< character font size */
#define FS_N	(1 << 3) /**< single or multiple lines */
#define FS_DL	(1 << 4) /**< 8/4 bit data interface */


//BUSY_FLAG
#define BF	(1 << 7) /**< busy/idle flag */

#define POWERON 1
#define POWEROFF 0


typedef enum e_darch {
    D8x1,
    D12x4,
    D16x0,
    D16x1,
    D16x2,
    D16x4,
    D20x2,
    D20x4,
    D24x2,
    D40x2,
} t_DisplayType;

/**
 * @brief Structure representing current state of an LCD.
 *
 */
typedef struct s_i2lcd {
    uint8_t bus; /**< I2C bus number */
    uint8_t address; /**< I2C chip address*/

    const uint8_t *ddramadr; /**< array of rows addresses */

    uint8_t buffer[4][40]; /**< temporary buffer for display */
    uint8_t commands[8]; /**< commands state buffer*/
    uint8_t cgbuff[8]; /**< current graphical character bitmap buffer*/
    uint8_t waitflag; /**< if 1, command function will wait until LCD is idle*/

    t_DisplayType dtype; /**< display type */

    uint8_t cols; /**< number of columns */
    uint8_t rows; /**< number of rows */
    uint8_t row; /**< current column, will change when lcdSetCursor() is used*/
    uint8_t column; /**< current row */

    uint8_t control; /**< current bit combination for CPORT*/

    t_Pca9535 iface; /**< PCA9535 interface structure */
    t_Potentiometer cpot; /**< Contrast potentiometer structure*/
    t_Potentiometer bpot; /**< Backlight brightness potentiometer structure*/

} t_I2Lcd;

/**
 * @brief Function behaves like printf(...) function, except it print its content on
 * LCD. __attribute__ part is GCC specific, so it may be not behave same on
 * different compiler.
 * @param *lcd addres of t_I2Lcd structure
 * @param *fmt format string
 * @param ... optional parameters, when fmt expect additional values
 */
void _lcdPrintf(t_I2Lcd *lcd, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
#define lcdPrintf(lcd, fmt, ...) _lcdPrintf(lcd, fmt, ##__VA_ARGS__)

/**
 * @brief Fill t_I2Lcd structure with proper values. Will open i2c device, set
 * width and height of an LCD.
 *
 * @param *lcd t_I2Lcd structure to fill up
 * @param bus I2C bus number
 * @param address I2C chip address
 * @param archit type of an LCD connected to the module
 */
void openI2LCD(t_I2Lcd *lcd, uint8_t bus, uint8_t address, t_DisplayType archit);

/**
 * @brief Free resources allocated by openI2LCD() function.
 * @param *lcd t_I2Lcd structure address
 */
void closeI2LCD(t_I2Lcd *lcd);

/**
 * @brief Set backlight intensity. Proper values are in range 0x00-0x3f
 * @param *lcd t_I2Lcd structure address
 * @param value brighness value
 */
void lcdSetBacklight(t_I2Lcd *lcd, uint8_t value);

/**
 * @brief Set LCD contrast. Proper values are in range 0x00-0x3f
 * @param *lcd t_I2Lcd structure address
 * @param value contrast value
 */
void lcdSetContrast(t_I2Lcd *lcd, uint8_t value);

/**
 * @brief Switch power on or off.
 * @param *lcd t_I2Lcd structure address
 * @param value
 */
void lcdPower(t_I2Lcd *lcd, uint8_t power);

/**
 * @brief LCD initialization commands. Function is called automatically
 * After Power OFF/ON cycle.
 * @param *lcd t_I2Lcd structure address
 * @return
 */
void lcdInit(t_I2Lcd *lcd);

/**
 * @brief Read current status of LCD operation. If BF in returned byte is set,
 * LCD is busy doing previous operation. Rest of the byte contains current
 * address of internal address counter for DDRAM or CGRAM, depends what operation
 * was previously requested.
 * @param *lcd t_I2Lcd structure address
 * @return byte with current dd/cgram address and BF
 */
uint8_t lcdReadStatus(t_I2Lcd *lcd);

/**
 * @brief Prints string to LCD as quick as possible. It doesn't respect any control
 * characters. It also print string without automatic line break and continuation
 * from the beginning of row below.
 * @param *lcd t_I2Lcd structure address
 */
void lcdFastPrint(t_I2Lcd *lcd, const char *string);

/**
 * @brief Prints string on LCD, it break line after "\\n" character, or if string don't
 * fit in line. It will continue to print string in row below if there's any,
 * or from the first row if current row was last one.
 * @param *lcd t_I2Lcd structure address
 */
void lcdPrint(t_I2Lcd *lcd, const char *string);

/**
 * @brief Will set bitmap for given graphical character. Those characters are the eight
 * character which user can redefine. Characters are numbered from 0 to 7
 * Bitmap is just simply 8 bytes of data defining how character will look.
 * @param *lcd t_I2Lcd structure address
 * @param *bitmap array of at lest 8 bytes
 */
void lcdSetGC(t_I2Lcd *lcd, uint8_t chr, const uint8_t *bitmap);

/**
 * @brief Function will return address of 8 byte buffer for graphical character given
 * by number. You shouldn't manipulate those bytes, but you can copy them to
 * your buffer to manipulate and send back by using lcdSetGC() function
 * @param *lcd t_I2Lcd structure address
 * @return address of 8 byte buffer with bitmap definition read from LCD
 */
const uint8_t *lcdGetGC(t_I2Lcd *lcd, uint8_t chr);

/**
 * @brief Clears LCD content, sending cursor to the top left corner of an LCD.
 * All characters are filled with 0x20 value.
 * @param *lcd t_I2Lcd structure address
 */
void lcdClear(t_I2Lcd *lcd);

/**
 * @brief Will move cursor to home position which is top left corner of an LCD.
 * @param *lcd t_I2Lcd structure address
 */
void lcdHome(t_I2Lcd *lcd);

/**
 * @brief Moves cursor to given position on the screen.
 * @param *lcd t_I2Lcd structure address
 * @param column column number of an LCD
 * @param row row number of an LCD
 */
void lcdSetCursor(t_I2Lcd *lcd, uint8_t column, uint8_t row);

/**
 * @brief Reads content of a given row number from LCD. Return address of a string
 * representing row content.
 * @param *lcd t_I2Lcd structure address
 * @param row row number
 * @return address of string
 */

const char *lcdReadRow(t_I2Lcd *lcd, uint8_t row);

/**
 * @brief Will switch blinking on or off depending on given value
 * @param *lcd t_I2Lcd structure address
 * @param value 0 off, 1 for on
 */
void lcdBlink(t_I2Lcd *lcd, uint8_t value);

/**
 * @brief Will set cursor visible or invisible depending on value
 * @param *lcd t_I2Lcd structure address
 * @param value 0 - switch cursor on, 1 - switch it off
 */
void lcdCursor(t_I2Lcd *lcd, uint8_t value);

/**
 * @brief Switch display on or off. If display is switched off, LCD will still be powered
 * on. Only content of DDRAM will be invisible. Content of DDRAM will also be
 * preserved. You can change content of an LCD, when it's switched off and after
 * switching it on, changes will appear on the screen.
 * @param *lcd t_I2Lcd structure address
 * @param value 0 - switch display off, 1 - swich display on
 */
void lcdDisplay(t_I2Lcd *lcd, uint8_t value);
#endif
