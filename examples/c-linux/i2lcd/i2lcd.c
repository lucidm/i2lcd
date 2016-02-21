#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <i2lcd.h>

static const uint8_t ddramadr[10][6] = {
    {8,  1, 0x00, 0x00, 0x00, 0x00},
    {12, 4, 0x00, 0x40, 0x0C, 0x4C}, //12x4
    {16, 1, 0x00, 0x00, 0x00, 0x00}, //16x1 type 1
    {8,  2, 0x00, 0x40, 0x00, 0x40}, //16x1 type 2 more popular
    {16, 2, 0x00, 0x40, 0x00, 0x40}, //16x2
    {16, 4, 0x00, 0x40, 0x10, 0x50}, //16x4
    {20, 2, 0x00, 0x40, 0x00, 0x40}, //20x2
    {20, 4, 0x00, 0x40, 0x14, 0x54}, //20x4
    {24, 2, 0x00, 0x40, 0x00, 0x40}, //24x2
    {40, 2, 0x00, 0x40, 0x00, 0x40}, //40x2
};

static inline void setControl(t_I2Lcd *lcd, uint8_t control, uint8_t value)
{
    lcd->control = value ? (lcd->control | control) : (lcd->control & (~control));
    setPortOutput(&lcd->iface, CPORT, lcd->control);
}

static inline void writeByte(t_I2Lcd *lcd, const char c)
{
    setControl(lcd, RS, 1);
    setControl(lcd, RW, 0);
    setControl(lcd, EN, 1);
    usleep(1);
    setPortOutput(&lcd->iface, DPORT, c);
    setControl(lcd, EN, 0);
    usleep(1);
    setControl(lcd, RS | RW | EN, 0);
}

static void writeBlock(t_I2Lcd *lcd, const char *block, uint8_t len)
{
    uint8_t i;

    setControl(lcd, RS, 1);
    setControl(lcd, RW, 0);
    for(i=0; i<len; i++)
    {
	setControl(lcd, EN, 1);
	usleep(1);
	setPortOutput(&lcd->iface, DPORT, block[i]);
	setControl(lcd, EN, 0);
	usleep(1);
    }
    setControl(lcd, RS | RW | EN, 0);
}


static void readBlock(t_I2Lcd *lcd, char *block, uint8_t len)
{
    uint8_t i;

    setControl(lcd, RS, 1);
    setControl(lcd, RW, 1);
    setPortDir(&lcd->iface, DPORT, 0xFF);
    for(i=0; i<len; i++)
    {
	setControl(lcd, EN, 1);
	block[i] = getPortInput(&lcd->iface, DPORT);
	usleep(1);
	setControl(lcd, EN, 0);
	usleep(1);
    }
    setPortDir(&lcd->iface, DPORT, 0x00);
    setControl(lcd, RS | RW | EN, 0);
}


static inline void command(t_I2Lcd *lcd, t_Command command, uint8_t value)
{
    uint8_t data = 0;

    while(lcd->waitflag && (lcdReadStatus(lcd) & BF)){};
    value |= (1 << (uint8_t) command);
    usleep(1);
    setControl(lcd, RS | RW, 0);
    usleep(1);
    setControl(lcd, EN, 1);
    usleep(1);
    setPortOutput(&lcd->iface, DPORT, value);
    usleep(2);
    setControl(lcd, EN, 0);
    lcd->commands[(uint8_t) command] = value;
}


void openI2LCD(t_I2Lcd *lcd, uint8_t bus, uint8_t address, t_DisplayType archit)
{
    openPCA9535(&lcd->iface, bus, address);

    lcd->control = 0x00;

    setPortDir(&lcd->iface, CPORT, IRS);
    setPortDir(&lcd->iface, DPORT, 0x00);
    setPortOutput(&lcd->iface, CPORT, (UD | BCS | CCS));

    openPotentiometer(&lcd->iface, &lcd->bpot, BCS, UD);
    openPotentiometer(&lcd->iface, &lcd->cpot, CCS, UD);

    lcd->control |= getPortOutput(&lcd->iface, CPORT);
    setPortOutput(&lcd->iface, CPORT, lcd->control);

    lcd->ddramadr = ddramadr[archit]+2;
    lcd->cols = ddramadr[archit][0];
    lcd->rows = ddramadr[archit][1];

    lcd->dtype = archit;

    lcd->column = 0;
    lcd->row = 0;

    lcd->bus = bus;
    lcd->address = address;
    lcd->waitflag = 0;

}

void closeI2LCD(t_I2Lcd *lcd)
{
    if (lcd)
    {
	setPot(&lcd->bpot, 0x00);
	setPot(&lcd->cpot, 0x00);
	setControl(lcd, EN | PWR, 0);
	setPortDir(&lcd->iface, CPORT, 0xFF);
	setPortDir(&lcd->iface, DPORT, 0xFF);
	closePotentiometer(&lcd->bpot);
	closePotentiometer(&lcd->cpot);
	closePCA9535(&lcd->iface);
    }
}

void lcdSetBacklight(t_I2Lcd *lcd, uint8_t value)
{
    setPot(&lcd->bpot, value);
}

void lcdSetContrast(t_I2Lcd *lcd, uint8_t value)
{
    setPot(&lcd->cpot, value);
}

void lcdPower(t_I2Lcd *lcd, uint8_t power)
{
    setControl(lcd, PWR, power);
    if (lcd->control & PWR)
	lcdInit(lcd);
    else
	lcd->waitflag = 0;
}


void lcdInit(t_I2Lcd *lcd)
{
    uint8_t fn = 0;

    if (lcd->rows > 1 || lcd->dtype == D16x0)
	fn = FS_N;

    usleep(4500);
    command(lcd, FUNCTION_SET, FS_DL | fn);
    usleep(5000);
    command(lcd, FUNCTION_SET, FS_DL | fn);
    usleep(4500);
    command(lcd, FUNCTION_SET, FS_DL | fn);
    usleep(4500);
    command(lcd, DISPLAY_ONOFF, 0x00);
    usleep(6000);
    command(lcd, CLEAR_DISPLAY, 0x00);
    usleep(30000);
    command(lcd, ENTRY_MODE_SET, EMS_ID);
    usleep(6000);
    command(lcd, DISPLAY_ONOFF, DOO_D);
    usleep(60000);
    lcd->waitflag = 1;
}

inline uint8_t lcdReadStatus(t_I2Lcd *lcd)
{
    uint8_t ret;

    setControl(lcd, RS, 0);
    setControl(lcd, RW, 1);
    setPortDir(&lcd->iface, DPORT, 0xFF);
    setControl(lcd, EN, 1);
    ret = getPortInput(&lcd->iface, DPORT);
    usleep(1);
    setControl(lcd, RS | RW | EN, 0);
    setPortDir(&lcd->iface, DPORT, 0x00);
    return ret;
}

void lcdFastPrint(t_I2Lcd *lcd, const char *string)
{
    uint8_t l;
    l = (lcd->column + strlen(string)) < lcd->cols ? strlen(string) : (lcd->cols - lcd->column);
    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[lcd->row] + lcd->column);
    writeBlock(lcd, string, l);
}

void lcdPrint(t_I2Lcd *lcd, const char *string)
{
    uint8_t column, row, i, c, l, idc = 0;

    column = lcd->column;
    row = lcd->row;

    l = strlen(string);
    for(i=0; i<l; i++)
    {
	c = *(string + i);

	if(c == '\n')
	{
	    //idc = 1;
	    row = (row + 1) % lcd->rows;
	    column = 0;
	    continue;
	} else
	{
	    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[row] + column);
	    writeByte(lcd, c);
	    column++;

	    if (column == lcd->cols)
	    {
	        if (idc == 0) idc = 1; //row = (row + 1) % lcd->rows;
		column = 0;
	    }
	}
	if (idc)
	{
	    row = (row + 1) % lcd->rows;
	    idc = 0;
	}
    }

    lcd->row = row;
    lcd->column = column < lcd->cols ? column : lcd->cols - 1;

    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[lcd->row] + lcd->column);
}

void _lcdPrintf(t_I2Lcd *lcd, const char *fmt, ...)
{
    va_list arg;
    char buf[100];

    va_start(arg, fmt);
    vsnprintf(buf, 100, fmt, arg);
    va_end(arg);
    lcdPrint(lcd, buf);
}

void lcdSetGC(t_I2Lcd *lcd, uint8_t chr, const uint8_t *bitmap)
{
    command(lcd, SET_CGRAM_ADDRESS, ((chr & 0x07) << 3));
    writeBlock(lcd, bitmap, 8);
}

const uint8_t *lcdGetGC(t_I2Lcd *lcd, uint8_t chr)
{
    command(lcd, SET_CGRAM_ADDRESS, ((chr & 0x07) << 3));
    readBlock(lcd, lcd->cgbuff, 8);
    return lcd->cgbuff;
}

void lcdClear(t_I2Lcd *lcd)
{
    command(lcd, CLEAR_DISPLAY, 0x00);
    lcd->column = 0;
    lcd->row = 0;
}

void lcdHome(t_I2Lcd *lcd)
{
    command(lcd, CURSOR_HOME, 0x00);
    lcd->column = 0;
    lcd->row = 0;
}

void lcdSetCursor(t_I2Lcd *lcd, uint8_t column, uint8_t row)
{
    column %= lcd->cols;
    row %= lcd->rows;
    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[row] + column);
    lcd->row = row;
    lcd->column = column;
}

const char* lcdReadRow(t_I2Lcd *lcd, uint8_t row)
{
    uint8_t i;

    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[row]);
    setControl(lcd, RS | RW, 1);
    setPortDir(&lcd->iface, DPORT, 0xFF);
    for(i=0; i<lcd->cols; i++)
    {
	usleep(1);
	setControl(lcd, EN, 1);
	lcd->buffer[row][i] = getPortInput(&lcd->iface, DPORT);
	usleep(1);
	setControl(lcd, EN, 0);
    }
    lcd->buffer[row][i] = 0;
    setControl(lcd, RS | RW, 0);
    setPortDir(&lcd->iface, DPORT, 0x00);
    command(lcd, SET_DDRAM_ADDRESS, lcd->ddramadr[lcd->row] + lcd->column);
    return (const char*) lcd->buffer[row];
}


void lcdBlink(t_I2Lcd *lcd, uint8_t value)
{
    command(lcd, DISPLAY_ONOFF,
            value ? (lcd->commands[DISPLAY_ONOFF] | DOO_B) : (lcd->commands[DISPLAY_ONOFF] & (~DOO_B)));
}

void lcdCursor(t_I2Lcd *lcd, uint8_t value)
{
    command(lcd, DISPLAY_ONOFF,
            value ? (lcd->commands[DISPLAY_ONOFF] | DOO_C) : (lcd->commands[DISPLAY_ONOFF] & (~DOO_C)));
}


void lcdDisplay(t_I2Lcd *lcd, uint8_t value)
{
    command(lcd, DISPLAY_ONOFF,
            value ? (lcd->commands[DISPLAY_ONOFF] | DOO_D) : (lcd->commands[DISPLAY_ONOFF] & (~DOO_D)));
}
