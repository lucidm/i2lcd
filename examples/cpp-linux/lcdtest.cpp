#include <iostream>
#include <i2lcd.h>

#include <unistd.h>

using namespace i2lcd;

int main(void)
{
    char c2[8] = {
		    0b00000,
		    0b01010,
		    0b11111,
		    0b11111,
		    0b01110,
		    0b00100,
		    0b00000,
		    0b00000,};


    I2Lcd lcd(2, 0x20, 16, 1);

    lcd.power(POWERON);
    lcd.setBacklight(0x3f);
    lcd.setContrast(0x08);
    lcd.setGC(2, c2);

    lcd.clear();

    lcd.setCursor(0, 0);
    //string s("HelloWorld");

    //lcd.print("Hello World!\nRow 1\nRow 2\nRow 3\n");
    lcd.print("Hello universe!\x02");

    //cout << lcd;

    lcd.blink(1);
    lcd.cursor(1);

    lcd._dump();

    usleep(3000 * 1000);
    lcd.power(POWEROFF);
}
