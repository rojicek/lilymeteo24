#ifndef SHOW_H
#define SHOW_H


#include "config.h"

void show_text(int x, int y, const unsigned char* font, String shown, String actual);
void drawPic(int x, int y, int dimx, int dim7, String pic);
void drawBox(int x, int y, int w, int h, uint16_t color);

#endif