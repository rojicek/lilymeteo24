
//#include <WiFi.h>
//#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>

//#include <ESP32Time.h>
//#include <ArduinoJson.h>
SPIClass* sdhander = nullptr;
//TTGOClass* ttgo;

#include "config.h"
#include "support.h"
//#include "wifi.h"

// jak casto sync hodiny s ntp
#define SYNC_CLOCK_SEC 60

//
//

//#include "ubuntu_fonts.h"

//#include "wifi.c"
//#include "support.c"



//ESP32Time board_time(0);  //no offset, board is syncd to local


// extra flag for sync time
unsigned long last_sync_time_epoch = 0;
unsigned long last_epoch_daily = 0;
unsigned long last_epoch_5min = 0;

unsigned long current_epoch = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("zacinam");

  // nastaveni obrazovky
  // ttgo = TTGOClass::getWatch();
  // ttgo->begin();
  // ttgo->openBL();
  // ttgo->tft->setRotation(3);
  // ttgo->tft->fillScreen(TFT_WHITE);

  /*
  if (wifi_connect() == 0) {  //wifi ok
    sync_local_clock();
    wifi_disconnect();
  } else {
    Serial.println("wifi failed");
  }
*/

  //SD karta
  /*
  if (sdcard_begin()) {
    Serial.println("sd ok");
  }
*/
  Serial.println("setup hotov");
  delay(1000);
}

void loop() {

  int do_anything = 0;
  int do_sync_clock = 0;

  //unsigned long current_epoch = board_time.getEpoch();
  if (current_epoch - last_sync_time_epoch > SYNC_CLOCK_SEC) {
    do_anything = 1;
    do_sync_clock = 1;
  }

  //////////////////////////////
  //executivni cast - jen jednou kvuli wifi
  /*
if (do_anything == 1) {
  if (wifi_connect() == 0) {  //wifi ok
   

    if (do_sync_clock == 1)
      sync_local_clock();

    wifi_disconnect();
  } 
}
*/

  // blbosti
  //String actual_time = board_time.getTime("%e %b, %R");

  //Serial.println(actual_time);

  Serial.println("----");
  delay(10000);
}