//http://www.rinkydinkelectronics.com/t_imageconverter565.php


#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>
#include <TimeLib.h>

SPIClass* sdhander = nullptr;


#include "config.h"
#include "support.h"
#include "wifi.h"
#include "show_elements.h"
#include "meteo.h"
#include "ubuntu_fonts.h"
#include "hourly_temp_screen.h"


TTGOClass* ttgo;

// jak casto sync hodiny s ntp (12 hodin nebo tak nejak)
#define SYNC_CLOCK_SEC 40000
// jak casto se nacte meteo (3 min)
#define QUICK_LOOP_SEC 180  //180

#define TXT_TIME_x 280
#define TXT_TIME_y 10


// XX cokoliv co nenastane
String shown_time("XX");

// extra flag for sync time
unsigned long last_sync_time_epoch = 0;
unsigned long last_quick_loop_epoch = 0;
unsigned long last_touch_epoch = 0;
int vsechno_prepis = -1;


void setup() {
  delay(5000);
  Serial.begin(115200);
  Serial.println("zacinam");

  // nastaveni obrazovky
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->tft->setRotation(3);
  ttgo->tft->fillScreen(TFT_SKYBLUE);

  int sd_count = 10;
  Serial.print("sd:");
  while ((sdcard_begin() != 0) && (sd_count > 0)) {
    Serial.print(".");
    sd_count--;
    delay(1000);
  }

  if (sd_count > 0)
    Serial.println(" ok!");
  else
    Serial.println(" failed!");

  //asi je to poprve stejne jedno
  vsechno_prepis = 1;

  delay(1000);  // nevim proc
  ttgo->tft->fillScreen(TFT_WHITE);
  Serial.println("setup hotov");
}

void loop() {


  ////////////////////

  int do_anything = 0;
  int do_sync_clock = 0;
  int do_quick_loop = 0;


  ESP32Time board_time(0);
  meteo_data md;


  unsigned long current_epoch = board_time.getEpoch();
  int secondOfDay = 3600 * board_time.getHour(true) + 60 * board_time.getMinute() + board_time.getSecond();

  if (ttgo->touched()) {
    last_touch_epoch = current_epoch;
  }

  //decide jestli ma displej svitit
  if ((secondOfDay > 1800) && (secondOfDay < 8 * 3600) && (current_epoch - last_touch_epoch > 60)) 
  {
    // v noci vypni displej ale musi to byt max 60s od posledniho dotyku
    ttgo->setBrightness(0);
    delay(100);
    return;
  }


  ttgo->setBrightness(200);

  // decide what do to now (later in wifi loop)
  if ((current_epoch - last_sync_time_epoch > SYNC_CLOCK_SEC) || (current_epoch < 1700000000)) {
    // druha cast == vim, ze cas je urcite blbe
    Serial.println("want to sync clock");
    last_sync_time_epoch = current_epoch;
    do_anything = 1;
    do_sync_clock = 1;
  }

  if ((current_epoch - last_quick_loop_epoch > QUICK_LOOP_SEC) || (current_epoch < 1700000000) || ((secondOfDay > 5) && (secondOfDay < 25) && (current_epoch - last_quick_loop_epoch > 22))) {
    // druha cast == vim, ze cas je urcite blbe
    // treti cast == 5-30s po pulnoci, ale min 10s po poslednim updatu (aby to nejelo furt do kola) - tohle by melo zabrat 1x hned po pulnoc (pockaa 5s na update serveru)
    Serial.println("want to quick loop");
    last_quick_loop_epoch = current_epoch;
    do_anything = 1;
    do_quick_loop = 1;
  }


  // kresli hodiny (uplne pokazde)
  String actual_time = board_time.getTime("%e %b, %R");
  show_text(TXT_TIME_x, TXT_TIME_x, TXT_TIME_y, ubuntu_regular_30, shown_time, actual_time);
  shown_time = actual_time;

  //////////////////////////////
  //executivni cast - jen jednou kvuli wifi

  // Serial.println("----");
  // Serial.print(actual_time);
  Serial.print(" --> epoch:");
  Serial.println(current_epoch);

  if (do_anything == 1) {
    drawBox(470, 310, 10, 10, TFT_BLUE);

    int vysledek_meteo_behu = 0;
    int vysledek_ntp_behu = 0;

    if (wifi_connect() == 0) {  //wifi ok

      if (do_sync_clock == 1) {
        Serial.println("do sync clock");
        vysledek_ntp_behu = sync_local_clock();
      }

      if (do_quick_loop == 1) {
        Serial.println("do quick loop");
        md = update_meteo();
        if (md.valid == true) {
          Serial.print("Vsechno prepis: ");
          Serial.println(vsechno_prepis);
          update_all_elements(md, vsechno_prepis);
          vsechno_prepis = 0;

          // vysledek_meteo_behu necham 0
        } else {
          vysledek_meteo_behu = 1;
        }
      }

      wifi_disconnect();

    } else {
      // neco se nepovedlo
      vysledek_meteo_behu = 2;
    }

    // neco se nepovedlo, zkratim cekani na dalsi pokus na minutu
    if (vysledek_meteo_behu != 0) {
      last_quick_loop_epoch = current_epoch - QUICK_LOOP_SEC + 60;
    }

    drawBox(470, 310, 10, 10, TFT_WHITE);
    kresli_info_ctverecek(vysledek_meteo_behu);
  }

  // pockam chvili nez vsechno zopakuji
  delay(100);

  int wait_loop = 5;
  int16_t touch_x, touch_y;
  while (wait_loop > 0) {
    // wait for push button
    if (ttgo->getTouch(touch_x, touch_y) == 1) {
      //if (1 == 1) {
      int new_screen = touch_screen_info(touch_x, touch_y);

      Serial.print("new screen id:");
      Serial.println(new_screen);

      if (new_screen == 1) {
        show_hourly_temp_screen();
        last_quick_loop_epoch = 0;  //enforce refresh
        vsechno_prepis = 1;
        shown_time = ".";  // prepis i cas
        //Serial.println("PREPSANO");
      }
    }

    wait_loop--;
    delay(80);

  }  //while wait loop
}