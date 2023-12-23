
#include <WiFi.h>
#include <HTTPClient.h>

#include <ESP32Time.h>
#include <ArduinoJson.h>

#include "config.h"


#include <SPI.h>
#include <SD.h>
//#include <FS.h>

#include "ubuntu_fonts.h"

TTGOClass* ttgo;
const char *wifi_ap = "R_host";
const char *wifi_pd = "badenka5";
ESP32Time board_time(0);  //no offset, board is syncd to local
SPIClass* sdhander = nullptr;


bool sdcard_begin() {
  if (!sdhander) {
    sdhander = new SPIClass(HSPI);
    pinMode(SD_MISO, INPUT_PULLUP);
    sdhander->begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  }
  if (!SD.begin(SD_CS, *sdhander)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->tft->setRotation(3);

  ttgo->tft->fillScreen(TFT_WHITE);

  //SD karta
  if (sdcard_begin()) {
    Serial.println("sd ok");
  } else {

    int cnt = 0;
    while (cnt < 1000) {
      Serial.println("sd failed");
      delay(1000);
      cnt++;
    }
  }

  Serial.println("setup hotov");
  delay(1000);
}

void loop() {
}