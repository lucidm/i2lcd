#include <Wire.h>
#include <LiquidCrystal_I2LCD.h>

#define CHIP_ADDRESS 0x20
LiquidCrystal_I2LCD i2lcd(CHIP_ADDRESS);

void setup() {

  Serial.begin(9600);
  Serial.println("--- LCD Initialization ---");
  
  // put your setup code here, to run once:
  i2lcd.begin(D16x2);
  i2lcd.setContrast(0x0c);
  i2lcd.clear();
  i2lcd.blink();
  i2lcd.cursor();
}

uint16_t oldac = 0;

void loop() {
   uint16_t acu = 0;
   for (int i=0; i<10; i++)
   {
    acu += analogRead(0);
    delay(25);
   }
   acu /= 10;
   acu = map(acu, 100, 800, 0x29, 0x3f);
   if (acu != oldac)
   {
      uint8_t diff;
      int8_t d;
      if (acu > oldac)
      {
        diff = acu - oldac;
        for(int i=0; i < diff; i++)
        {
          i2lcd.setBacklight(oldac++);
          delay(75);
        }
      } else if (acu < oldac)
      {
        diff = oldac - acu;
        for(int i=0; i < diff; i++)
        {
          i2lcd.setBacklight(oldac--);
          delay(75);
        }
      }
   }
   oldac = acu;
      
   i2lcd.setCursor(0, 0);
   i2lcd.print("Light sensor");
   i2lcd.setCursor(0, 1);
   i2lcd.print(acu);
   Serial.println(acu);
   delay(200);
}
