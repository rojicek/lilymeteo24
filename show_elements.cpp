#include "show_elements.h"
#include "ubuntu_fonts.h"

extern TTGOClass* ttgo;

void show_text(int x, int y, const unsigned char* font, String shown, String actual)
{
  if (shown != actual)
  {
    ttgo->tft->loadFont(font);
    ttgo->tft->setTextColor(TFT_WHITE);
    ttgo->tft->setCursor(x, y);
    ttgo->tft->print(shown);
    ttgo->tft->setTextColor(TFT_BLACK);
    ttgo->tft->setCursor(x, y);
    ttgo->tft->print(actual);
  }
  return;
}