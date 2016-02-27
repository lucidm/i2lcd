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


    I2Lcd lcd(2, 0x20, 16, 2);

    lcd.power(POWERON);
    lcd.setBacklight(0x3f);
    lcd.setContrast(0x17);
    lcd.setGC(2, c2);
    lcd.blink(1);
    lcd.cursor(1);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Hello universe!");

    lcd._dump();

    usleep(3000 * 1000);
    lcd.power(POWEROFF);
}
