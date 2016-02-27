#include <i2lcd.h>
#include <pca9535.h>
#include <pots.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define BUS 2
#define ADDRESS 0x20

uint8_t quit;

void shandler(int signo)
{
    if (signo == SIGINT)
	quit = 1;
}

void brght(t_I2Lcd *lcd, uint8_t v)
{
    if(quit) return;
    lcdSetCursor(lcd, 0, 0);
    lcdPrintf(lcd, "Brightness 0x%02X", v);
    lcdSetBacklight(lcd, v);
    usleep(50000);
}


void ctrst(t_I2Lcd *lcd, uint8_t v)
{
    if(quit) return;
    lcdSetCursor(lcd, 0, 1);
    lcdPrintf(lcd, "Contrast 0x%02X\x02", v);
    lcdSetContrast(lcd, v);
    usleep(50000);
}


int main(void)
{
    t_I2Lcd lcd;

    unsigned long ts;
    int i;
    char b[255], c;

    quit = 0;

    if (signal(SIGINT, shandler) == SIG_ERR)
	printf("Cant install SIGINT handler!\n");

    uint8_t c2[8] = {
		    0b00000,
		    0b01010,
		    0b11111,
		    0b11111,
		    0b01110,
		    0b00100,
		    0b00000,
		    0b00000,};


    openI2LCD2(&lcd, BUS, ADDRESS, 16, 2);
    lcdPower(&lcd, POWERON);
    lcdBlink(&lcd, 1);
    lcdCursor(&lcd, 1);
    lcdClear(&lcd);
    lcdSetCursor(&lcd, 0, 0);
    lcdSetGC(&lcd, 2, c2);

    brght(&lcd, 0x3f);
    ctrst(&lcd, 0x17);

    do
    {
	for(i=0; i < 0x40; i++)
	{
	    brght(&lcd, i);
	}

	for(i=0; i < 0x40; i++)
	{
	    ctrst(&lcd, i);
	}
	ctrst(&lcd, 0x17);

    }while(!quit);
    closeI2LCD(&lcd);
}
