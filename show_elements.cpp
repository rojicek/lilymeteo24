#include "show_elements.h"
#include "ubuntu_fonts.h"


#define drawPixel(a, b, c) \
  ttgo->tft->setAddrWindow(a, b, a, b); \
  ttgo->tft->pushColor(c)


extern TTGOClass* ttgo;

void show_text(int past_x, int x, int y, const unsigned char* font, String shown, String actual) {
  if (shown != actual) {
    ttgo->tft->loadFont(font);
    ttgo->tft->setTextColor(TFT_WHITE);
    ttgo->tft->setCursor(past_x, y);
    ttgo->tft->print(shown);
    ttgo->tft->setTextColor(TFT_BLACK);
    ttgo->tft->setCursor(x, y);
    ttgo->tft->print(actual);
  }
  return;
}



void drawPic(int x, int y, int dimx, int dimy, String pic) {

  File picFile = SD.open(pic);
  //Serial.print("soubor handler:>");
  //Serial.print(picFile);
  //Serial.println("<<<");


  if (picFile == 1) {

    uint8_t* pbuffer = nullptr;
    pbuffer = (uint8_t*)malloc(dimx * dimy * 2);

    picFile.read(pbuffer, dimx * dimy * 2);

    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {

        int ix = 2 * ((i - x) + dimx * (j - y));

        drawPixel(i, j, 256 * pbuffer[ix] + pbuffer[ix + 1]);
      }
    }

    if (pbuffer) free(pbuffer);

    //Serial.print(pic);
    //Serial.println(".. ok!");
  } else {
    Serial.print(pic);
    Serial.println(".. not found");

    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {

        int ix = 2 * ((i - x) + dimx * (j - y));

        drawPixel(i, j, TFT_WHITE);
      }
    }
  }

  close(picFile);
}

void drawBox(int x, int y, int w, int h, uint16_t color) {

  for (int i = x; i < x + w; i++) {
    for (int j = y; j < y + h; j++) {
      int ix = 2 * (w * (i - x) + (j - y));  //counter

      drawPixel(i, j, color);
    }
  }
}
