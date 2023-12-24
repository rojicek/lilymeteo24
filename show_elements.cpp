#include "show_elements.h"
#include "ubuntu_fonts.h"

extern TTGOClass* ttgo;

void show_clock(String shown_time, String actual_time) {

  if (shown_time != actual_time) {
    ttgo->tft->loadFont(ubuntu_regular_30);
    ttgo->tft->setTextColor(TFT_WHITE);
    ttgo->tft->setCursor(280, 10);
    ttgo->tft->print(shown_time);
    ttgo->tft->setTextColor(TFT_BLACK);
    ttgo->tft->setCursor(280, 10);
    ttgo->tft->print(actual_time);
  }
  return;
}