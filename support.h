#ifndef SUPPORT_H
#define SUPPORT_H

#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>
#include "config.h"

int sdcard_begin();
void sync_local_clock();

#endif