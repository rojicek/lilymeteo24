#include <SD.h>
#include <SPI.h>

#include "support.h"

extern SPIClass* sdhander;

int sdcard_begin() {

  if (!sdhander) {
    sdhander = new SPIClass(HSPI);
    pinMode(SD_MISO, INPUT_PULLUP);
    sdhander->begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  }

  if (!SD.begin(SD_CS, *sdhander)) {
    Serial.println("SD Card Mount Failed");
    return 1;
  }

  return 0;
}