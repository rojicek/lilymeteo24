

#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>

SPIClass* sdhander = nullptr;



#include "config.h"
#include "support.h"
#include "wifi.h"
#include "show_elements.h"
#include "meteo.h"
#include "ubuntu_fonts.h"


TTGOClass* ttgo;

// jak casto sync hodiny s ntp ()
#define SYNC_CLOCK_SEC 60
// jak casto se nacte meteo (15min)
#define QUICK_LOOP_SEC 60


#define TXT_TIME_x 280
#define TXT_TIME_y 10

#define TXT_SUNRISE_x 10
#define TXT_SUNRISE_y 10
#define TXT_SUNSET_x 130
#define TXT_SUNSET_y 10

#define ICON_SUN_x 80
#define ICON_SUN_y 0
#define ICON_SUN_width 40
#define ICON_SUN_height 40

#define ICON_WEATHER_x 30
#define ICON_WEATHER_y 50
#define ICON_WEATHER_width 200
#define ICON_WEATHER_height 150

#define TXT_OAT_x 70
#define TXT_OAT_y 210

// XX je cokoliv co nenastane
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
  delay(5000);
  ttgo->tft->fillScreen(TFT_WHITE);
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


  // kresli hodiny (uplne pokazde)
  String actual_time = board_time.getTime("%e %b, %R");
  show_text(TXT_TIME_x, TXT_TIME_y, ubuntu_regular_30, shown_time, actual_time);
  shown_time = actual_time;


  //////////////////////////////
  //executivni cast - jen jednou kvuli wifi

  Serial.println("----");
  Serial.println(actual_time);

  if (do_anything == 1) {
    if (wifi_connect() == 0) {  //wifi ok

      if (do_sync_clock == 1) {
        Serial.println("do sync clock");
        sync_local_clock();
      }


      if (do_quick_loop == 1) {
        Serial.println("do quick loop");
        md = update_meteo();
        if (md.valid == true) {

          String actual_sunrise = md.sunrise;
          show_text(TXT_SUNRISE_x, TXT_SUNRISE_y, ubuntu_regular_30, shown_sunrise, actual_sunrise);
          shown_sunrise = actual_sunrise;

          String actual_sunset = md.sunset;
          show_text(TXT_SUNSET_x, TXT_SUNSET_y, ubuntu_regular_30, shown_sunset, actual_sunset);
          shown_sunset = actual_sunset;

          if (md.sunlight == 0)
            drawPic(ICON_SUN_x, ICON_SUN_y, ICON_SUN_width, ICON_SUN_height,  "/meteo/sun/sunset-square40.raw");
          else
            drawPic(ICON_SUN_x, ICON_SUN_y, ICON_SUN_width, ICON_SUN_height, "/meteo/sun/sunrise-square40.raw");


          drawPic(ICON_WEATHER_x, ICON_WEATHER_y, ICON_WEATHER_width, ICON_WEATHER_height, "/meteo/weather/04n.raw");

          //drawBox(30, 50, 200, 150, TFT_SKYBLUE);

          char actual_oat[5];
          sprintf(actual_oat, "%d°C", md.oat);
          show_text(TXT_OAT_x, TXT_OAT_y, ubuntu_bold_45, shown_oat, actual_oat);
          shown_oat = actual_oat;

          show_text(70, 260, ubuntu_regular_30, "+2C", "+3C");
          //drawPic(50, 260, 30, 30, "/trend_small_up.raw");  //temp trend
          //drawBox(50, 260, 40, 40, TFT_SKYBLUE);
          drawPic(40, 260, 40, 40, "/meteo/trend/trend_small_up_40.raw");
          



          Serial.println(shown_oat);
          Serial.println(shown_sunrise);
          Serial.println(shown_sunset);
        }
      }

      wifi_disconnect();
    }
  }


  delay(10000);
}