#ifndef MENU_H
#define MENU_H

#include <U8g2lib.h>
#include "settings.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Holds menu state, and navigation and drawing functions
class Menu {
public:
  Menu(Settings* s);
  void begin();
  void drawBatteryVoltage(int voltage);

private:
  int menuIndex;
  Settings* settings;

  // If using an OLED with an SH1106 chip then leave this be
  // If using an OLED with an SSD1306 chip then comment out the SH1106 line and uncomment the SSD1306 line
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
  // U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
};

#endif
