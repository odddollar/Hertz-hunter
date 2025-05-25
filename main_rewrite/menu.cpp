#include "menu.h"

Menu::Menu(Settings *s)
  : menuIndex(0), settings(s), u8g2(U8G2_R0, U8X8_PIN_NONE) {
}

// Begin display library
void Menu::begin() {
  u8g2.begin();
  u8g2.clearBuffer();
}

// Display battery voltage in bottom corner of main menu
void Menu::drawBatteryVoltage(int voltage) {
  // Only display if on main menu
  if (menuIndex == 0) {
    // Format voltage reading
    char formattedVoltage[5];
    snprintf(formattedVoltage, sizeof(formattedVoltage), "%d.%dv", voltage / 10, voltage % 10);

    // Set font colour to inverted if selected bottom item
    u8g2.setDrawColor(menuIndex == 2 ? 0 : 1);
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(109, DISPLAY_HEIGHT, formattedVoltage);
    u8g2.setDrawColor(1);

    // Send drawing to display
    u8g2.sendBuffer();
  }
}
