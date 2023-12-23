
#include <WiFi.h>
#include <HTTPClient.h>

#include <ESP32Time.h>
#include <ArduinoJson.h>

#include "config.h"


#include <SPI.h>
#include <SD.h>

#include "ubuntu_fonts.h"

#include "wifi.c"

TTGOClass* ttgo;

ESP32Time board_time(0);  //no offset, board is syncd to local
SPIClass* sdhander = nullptr;


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("zacinam");

  // nastaveni obrazovky
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->tft->setRotation(3);
  ttgo->tft->fillScreen(TFT_WHITE);


  //SD karta
  if (sdcard_begin()) {
    Serial.println("sd ok");
  }

  Serial.println("setup hotov");
  delay(1000);
}

void loop() {

  Serial.println("x smycka");
  delay(1000);
}