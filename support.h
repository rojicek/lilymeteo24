#ifndef SUPPORT_H
#define SUPPORT_H

#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>
#include "config.h"

int sdcard_begin();
int sync_local_clock();
int touch_screen_info(int16_t x, int16_t y);

#endif