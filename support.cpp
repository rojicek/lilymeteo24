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


// Function that gets current epoch time
void sync_local_clock() {
  
  const char *ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 3600;
  const int daylightOffset_sec = 3600;

  ESP32Time board_time(0); 

  //sync with NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  time_t now;
  struct tm timeinfo;
  int count = 10;  //how many tries
  while (count > 0) {
    if (getLocalTime(&timeinfo)) {
      count = -1;
      Serial.println("NTP time syncd");
    } else {
      count--;  //try again
      Serial.println("NTP time failed");
      delay(500);
    }
  }
  time(&now);

  board_time.setTime(now);
  
  return;
}

