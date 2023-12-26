//http://www.rinkydinkelectronics.com/t_imageconverter565.php 


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
#define TXT_OAT_y 215

#define TXT_TREND_TEMP_x 113
#define TXT_TREND_TEMP_y 269

#define ICON_TREND_x 63
#define ICON_TREND_y 264
#define ICON_TREND_width 40
#define ICON_TREND_height 40


// XX je cokoliv co nenastane
String shown_time("XX");
String shown_sunrise("XX");
String shown_sunset("XX");
String shown_oat("XX");
String shown_trend_temp("XX");
int shown_past_shift_x = 0;



// extra flag for sync time
unsigned long last_sync_time_epoch = 0;
unsigned long last_quick_loop_epoch = 0;


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

 // strcpy(shown_oat, "xx");
 // strcpy(shown_trend_temp, "xx");
 
  //SD karta
  if (sdcard_begin()) {
    Serial.println("sd ok");
  }
  else
  {
     Serial.println("sd failed");
  }
  

  delay(5000);
  ttgo->tft->fillScreen(TFT_WHITE);
  Serial.println("setup hotov");
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

  if ((current_epoch - last_quick_loop_epoch > QUICK_LOOP_SEC) || (current_epoch < 1700000000)) {
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
          show_text(TXT_SUNRISE_x, TXT_SUNRISE_x, TXT_SUNRISE_y, ubuntu_regular_30, shown_sunrise, actual_sunrise);
          shown_sunrise = actual_sunrise;

          String actual_sunset = md.sunset;
          show_text(TXT_SUNSET_x, TXT_SUNSET_x, TXT_SUNSET_y, ubuntu_regular_30, shown_sunset, actual_sunset);
          shown_sunset = actual_sunset;

          if (md.sunlight == 0)
            drawPic(ICON_SUN_x, ICON_SUN_y, ICON_SUN_width, ICON_SUN_height, "/meteo/sun/sunset-square40.raw");
          else
            drawPic(ICON_SUN_x, ICON_SUN_y, ICON_SUN_width, ICON_SUN_height, "/meteo/sun/sunrise-square40.raw");


          //drawBox(30, 50, 200, 150, TFT_SKYBLUE); //show icon box
          //prekreslim pokazde - je to jedno
          char w_icon_path[22];
          strcpy(w_icon_path, "/meteo/weather/");
          strcat(w_icon_path, md.w_icon);
          strcat(w_icon_path, ".raw");
          drawPic(ICON_WEATHER_x, ICON_WEATHER_y, ICON_WEATHER_width, ICON_WEATHER_height, w_icon_path);


          String actual_oat("");  
          actual_oat = (String)md.oat + "°C";
         // sprintf(actual_oat, "%d°C", md.oat);
          int x_shift = 0;
          if (md.oat <= -10)
            x_shift = -10;
          if ((md.oat >= 0) && (md.oat < 10))
            x_shift = 15;
          // pod -10 : -10px
          // -10 .. 0: default
          // 0 .. 10: +15px
          // nad +10: default  

          show_text(TXT_OAT_x+shown_past_shift_x, TXT_OAT_x+x_shift, TXT_OAT_y, ubuntu_bold_45, shown_oat, actual_oat);
          shown_oat = actual_oat;
          shown_past_shift_x = x_shift;

          String actual_trend_temp("");  
          actual_trend_temp = String(md.trend_temp) + "°C";
          show_text(TXT_TREND_TEMP_x, TXT_TREND_TEMP_x, TXT_TREND_TEMP_y, ubuntu_regular_30, shown_trend_temp, actual_trend_temp);
          shown_trend_temp = actual_trend_temp;

          //prekreslim pokazde - je to jedno
          String full_path_trend_icon = "/meteo/trend/" + String(md.trend_icon) + "_40.raw";
          drawPic(ICON_TREND_x, ICON_TREND_y, ICON_TREND_width, ICON_TREND_height, full_path_trend_icon);   


          show_text(250, 250, 50, ubuntu_regular_30, "x", "now");
          show_text(360, 360, 50, ubuntu_regular_30, "x", "thu");
          drawPic(260, 110, 75, 75, "/cycle_2.raw");
          drawPic(360, 110, 75, 75, "/cycle_2.raw");


          Serial.println(shown_oat);
          Serial.print("Sunrise:");
          Serial.println(shown_sunrise);
          Serial.print("Sunset:");
          Serial.println(shown_sunset);
          Serial.println(full_path_trend_icon);
        }
      }

      wifi_disconnect();
    }
  }


  delay(10000);
}