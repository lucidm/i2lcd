#include <i2lcd.h>
#include <pca9535.h>
#include <pots.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define BUS 2
#define ADDRESS 0x20

#define DEGRAD (M_PI/180.0)

void dumplcd(t_I2Lcd *lcd)
{
    uint8_t i;

    printf("+");
    for(i=0; i < lcd->cols; i++) printf("-");
    printf("+\n");
    for(i=0; i < lcd->rows; i++)
        printf("|%s|\n", lcdReadRow(lcd, i), "|");
    printf("+");
    for(i=0; i < lcd->cols; i++) printf("-");
    printf("+\n");
}

uint32_t getts()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

int main(void)
{
    t_I2Lcd lcd;

    unsigned long ts;
    int i;
    char b[255], c;

    uint8_t c2[8] = {
		    0b00000,
		    0b01010,
		    0b11111,
		    0b11111,
		    0b01110,
		    0b00100,
		    0b00000,
		    0b00000,};


    ts = getts();

    openI2LCD(&lcd, BUS, ADDRESS, D16x2);
    lcdPower(&lcd, POWERON);
    lcdSetBacklight(&lcd, 0x3f);
    lcdSetContrast(&lcd, 0x0a);
    lcdBlink(&lcd, 1);
    lcdCursor(&lcd, 1);

    lcdClear(&lcd);

    lcdSetCursor(&lcd, 0, 0);
    lcdSetGC(&lcd, 2, c2);

    ts = getts();
    lcdPrintf(&lcd, "Hello World!");
    ts = getts() - ts;

    lcdSetCursor(&lcd, 0, 1);
    lcdPrintf(&lcd, "Line No:%d", __LINE__);

    dumplcd(&lcd);

    for(i=0; i<450; i++)
    {
        lcdSetBacklight(&lcd, (36 + 27 * sin((float)i * DEGRAD)));
        usleep(10000);
    }

    usleep(3000 * 1000);

    lcdPower(&lcd, POWEROFF);
    closeI2LCD(&lcd);

    printf("Time spent on LCD print function: %f\n", ts / 1000000.0);
}
