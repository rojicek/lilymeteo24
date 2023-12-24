
//#include <WiFi.h>
//#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>
//#include <ArduinoJson.h>
SPIClass* sdhander = nullptr;



#include "config.h"
#include "support.h"
#include "wifi.h"
#include "show_elements.h"
#include "meteo.h"
#include "ubuntu_fonts.h"


TTGOClass* ttgo;

// jak casto sync hodiny s ntp
#define SYNC_CLOCK_SEC 60
#define QUICK_LOOP_SEC 60

String shown_time("XX");
String shown_sunrise("XX");
String shown_sunset("XX");
String shown_oat("XX");


// extra flag for sync time
unsigned long last_sync_time_epoch = 0;
unsigned long last_quick_loop_epoch = 0;


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

  int do_anything = 0;
  int do_sync_clock = 0;
  int do_quick_loop = 0;

  ESP32Time board_time(0);
  meteo_data md;

  unsigned long current_epoch = board_time.getEpoch();

  // decide what do to now (if wifi loop)
  if ((current_epoch - last_sync_time_epoch > SYNC_CLOCK_SEC) || (current_epoch < 1700000000)) {
    // druha cast == vim, ze cas je urcite blbe
    Serial.println("want to sync clock");
    last_sync_time_epoch = current_epoch;
    do_anything = 1;
    do_sync_clock = 1;
  }

  if (current_epoch - last_quick_loop_epoch > QUICK_LOOP_SEC) {
    Serial.println("want to quick loop");
    last_quick_loop_epoch = current_epoch;
    do_anything = 1;
    do_quick_loop = 1;
  }



  //////////////////////////////
  //executivni cast - jen jednou kvuli wifi

  if (do_anything == 1) {
    if (wifi_connect() == 0) {  //wifi ok

      if (do_sync_clock == 1) {
        Serial.println("do sync clock");
        sync_local_clock();
      }


      if (do_quick_loop == 1) {
        Serial.println("do quick loop");
        md = update_meteo();
        if (md.valid == true)
        {
          char actual_oat[5];
          sprintf(actual_oat, "%d°C", md.oat);  
          show_text(50, 200, ubuntu_bold_45, shown_oat, actual_oat);
          shown_oat = actual_oat;

          String actual_sunrise = md.sunrise;
          show_text(10, 10, ubuntu_regular_30, shown_sunrise, actual_sunrise);
          shown_sunrise = actual_sunrise;

          //String actual_sunset = md.sunset;
          //show_text(70, 10, ubuntu_regular_30, shown_sunset, actual_sunset);
          //shown_sunset = actual_sunset;

          Serial.println(shown_oat);
          Serial.println(shown_sunrise);
           //Serial.println(shown_sunset);
        }
      }

      wifi_disconnect();
    }
  }


// kresli hodiny (uplne pokazde)
  String actual_time = board_time.getTime("%e %b, %R");
  show_text(280, 10, ubuntu_regular_30, shown_time, actual_time);
  shown_time = actual_time;

  




  // blbosti

  Serial.println(actual_time);
  Serial.println("----");
  delay(10000);
}