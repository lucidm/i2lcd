#include <Wire.h>
#include <LiquidCrystal_I2LCD.h>

#define CHIP_ADDRESS 0x20
LiquidCrystal_I2LCD i2lcd(CHIP_ADDRESS);
uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000,};

void setup() {

  Serial.begin(9600);
  Serial.println("--- LCD Initialization ---");
  
  // put your setup code here, to run once:
  i2lcd.begin(D16x2);
  i2lcd.setBacklight(0x3f);
  i2lcd.setContrast(0x0c);
  i2lcd.clear();
  i2lcd.blink();
  i2lcd.cursor();
  i2lcd.autoscroll();
  i2lcd.createChar(2, heart);
  
  i2lcd.setCursor(0, 0);
  i2lcd.print("Hello World!");
  i2lcd.write((char)'\x02');
  i2lcd.setCursor(0, 1);
  i2lcd.print("I2LCD Library");
  
}

void loop() {
   i2lcd.setCursor(0, 0);
   i2lcd.print("Light sens:");
   i2lcd.print(analogRead(0));
   delay(250);
}
