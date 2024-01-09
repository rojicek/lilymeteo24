

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>
#include <ESP32Time.h>

#include "meteo.h"
#include "ubuntu_fonts.h"
#include "show_elements.h"

#define DELKA_PROGRAMU 9000  // sekundy
#define RANNI_HDO_HODINA 9   // druhe hdo chci co nejblize tolika hodin rano (konec programu)



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
#define TXT_TREND_TEMP_y 270

#define ICON_TREND_x 63
#define ICON_TREND_y 264
#define ICON_TREND_width 40
#define ICON_TREND_height 40

// XX je cokoliv co nenastane
String shown_sunrise("XX");
String shown_sunset("XX");
String shown_oat("XX");
String shown_trend_temp("XX");
String shown_tomorrow("XX");
String shown_today("xx");
String shown_hdo("xx");
int shown_past_shift_x = 0;

String day_of_week[7] = { "sun", "mon", "tue", "wed", "thu", "fri", "sat" };


int kolik_je_prunik(unsigned long start1, unsigned long end1, unsigned long start2, unsigned long end2) {
  if (start1 > start2) {
    //prohodit
    unsigned long tmp = start1;
    start1 = start2;
    start2 = tmp;

    tmp = end1;
    end1 = end2;
    end2 = tmp;
  }

  // ted vim, ze start1 je prvni
  if (start2 >= end1)
    return 0;
  else {
    if (end2 <= end1)  // druhy je cely v prvnim
      return end2 - start2;
    else
      return end1 - start2;
  }
}


meteo_data update_meteo() {
  meteo_data md;

  StaticJsonDocument<1000> w_doc;
  HTTPClient http;
  String url = "https://www.rojicek.cz/meteo/meteo-query.php?pwd=pa1e2";

  http.begin(url.c_str());

  int httpResponseCode = http.GET();


  if (httpResponseCode == 200) {
    Serial.println("meteo query ok");
    String payload = http.getString();
    DeserializationError error = deserializeJson(w_doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      md.valid = false;
      return md;
    }


    md.oat = w_doc["weather"]["temp"];
    //  md.oat = (int)random(-15, 35);  // debug

    time_t sunrise = w_doc["weather"]["sunrise"];
    struct tm sunrise_local = *localtime(&sunrise);
    strftime(md.sunrise, sizeof(md.sunrise), "%H:%M", &sunrise_local);

    //Serial.print("raw sunrise:");
    //Serial.println(sunrise);

    // odstran nulu na pocatku vychodu slunce. if je zbytecne, ale radeji jo
    if (md.sunrise[0] == '0')
      memmove(md.sunrise, md.sunrise + 1, strlen(md.sunrise));

    time_t sunset = w_doc["weather"]["sunset"];
    struct tm sunset_local = *localtime(&sunset);
    strftime(md.sunset, sizeof(md.sunset), "%H:%M", &sunset_local);

    //Serial.print("raw sunset:");
    //Serial.println(sunset);

    // decide if its day or night
    ESP32Time board_time(0);
    unsigned long current_epoch = board_time.getEpoch();
    if ((current_epoch > w_doc["weather"]["sunrise"]) && (current_epoch < w_doc["weather"]["sunset"]))
      md.sunlight = 1;
    else
      md.sunlight = 0;

    strcpy(md.w_icon, w_doc["weather"]["w_icon"]);
    strcpy(md.trend_icon, w_doc["weather"]["temp_trend_icon"]);

    md.trend_temp = w_doc["weather"]["temp_trend"];
    //debug!!
    //md.trend_temp = (int)random(-12, 12);

    md.clc_tdy = w_doc["weather"]["clc_tdy"];
    md.clc_tmr = w_doc["weather"]["clc_tmr"];

    strcpy(md.aqi, w_doc["weather"]["aqi"]);

    //get hdo info
    md.hdo1 = -1;
    md.hdo2 = -1;

    JsonArray hdo_arr = w_doc["hdo_intervals"].as<JsonArray>();
    // uz mam current_epoch
    // smycka pro hodiny, ktere testuji

    //  Serial.println(hdo_arr[0][0]);
    //  Serial.print("*->");

    struct tm ranni_hdo;
    // CHYBA - chci nejblizsi 9h rano
    time_t ranni_hdo_seconds = current_epoch;  // + 86400;
    
    if (board_time.getHour(true) > RANNI_HDO_HODINA) //pokud uz je po 9 rano, tak dalsi bude az zitra - prictu den
      ranni_hdo_seconds = ranni_hdo_seconds + 86400;

    memcpy(&ranni_hdo, localtime(&ranni_hdo_seconds), sizeof(struct tm));
    ranni_hdo.tm_hour = RANNI_HDO_HODINA; // nastavim hodinu natvrdo
    ranni_hdo.tm_min = 0; // smazu hodiny
    ranni_hdo.tm_sec = 0; // a sekundy

    unsigned long ranni_hdo_epoch = mktime(&ranni_hdo);
    unsigned long time_from_ranni_hdo = 999999999;
    int prev_hdo2_ok = 0;


    for (int one_hour = 0; one_hour < 24; one_hour++) {

      // Serial.println("----------------------------------");
      // Serial.print("one hour:");
      // Serial.println(one_hour);

      unsigned long one_start_epoch = current_epoch + one_hour * 3600;
      unsigned long one_end_epoch = DELKA_PROGRAMU + one_start_epoch;

      int ve_drahe_sazbe = 0;



      for (JsonVariant value : hdo_arr) {
        ve_drahe_sazbe += kolik_je_prunik(one_start_epoch, one_end_epoch, value[0].as<const int>(), value[1].as<const int>());
      }


      if ((float)ve_drahe_sazbe / (float)DELKA_PROGRAMU < 0.05) {
        // tahle hodina je ok pro prani (mene nez 3 procenta)
        if (md.hdo1 < 0) {
          // prvni vezmu do hdo1
          md.hdo1 = one_hour;
        }


        if (md.hdo2 < 0) {
          long hdo2_time_diff = (long)(one_end_epoch - ranni_hdo_epoch);
          if (hdo2_time_diff < 0)
            hdo2_time_diff = -hdo2_time_diff;  //abs

          if (hdo2_time_diff < time_from_ranni_hdo) {
            time_from_ranni_hdo = hdo2_time_diff;
          } else {
            if (one_hour == 0) {
              md.hdo2 = 0;
            } else {
              md.hdo2 = prev_hdo2_ok;
              // Serial.print("beru:");
              // Serial.println(prev_hdo2_ok);
            }
          }
        }
        // zapisu si posledni hodinu, kdy muzu spustit program
        //Serial.println("zapisuji prev_hdo2_ok");
        prev_hdo2_ok = one_hour;
      }
    }

    //tohle tu nejak patri, ale uplne to promyslene nemam - pouzije se, kdyz je spravna ta posledni hodina
    if (md.hdo2 < 0)
      md.hdo2 = prev_hdo2_ok;

    //Serial.println("<<<array");


    md.valid = true;
    Serial.println("meteo updated");
  } else {
    Serial.print("meteo query returned:");
    Serial.println(httpResponseCode);
    md.valid = false;
  }


  return md;
}

void update_all_elements(meteo_data md, int vsechno_prepis) {

  if (vsechno_prepis == 1) {
    //
    shown_sunrise = "XX";
    shown_sunset = "XX";
    shown_oat = "XX";
    shown_trend_temp = "XX";
    shown_tomorrow = "XX";
    shown_today = "xx";
    shown_hdo = "xx";
    shown_past_shift_x = 0;
  }


  ESP32Time board_time(0);
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

  show_text(TXT_OAT_x + shown_past_shift_x, TXT_OAT_x + x_shift, TXT_OAT_y, ubuntu_bold_45, shown_oat, actual_oat);
  shown_oat = actual_oat;
  shown_past_shift_x = x_shift;

  String actual_trend_temp("");
  if (md.trend_temp > 0)
    actual_trend_temp = "+";
  else
    actual_trend_temp = "";  //minus je v cisle
  actual_trend_temp = actual_trend_temp + String(md.trend_temp) + "°C";
  show_text(TXT_TREND_TEMP_x, TXT_TREND_TEMP_x, TXT_TREND_TEMP_y, ubuntu_regular_30, shown_trend_temp, actual_trend_temp);
  shown_trend_temp = actual_trend_temp;

  //prekreslim pokazde - je to jedno
  String full_path_trend_icon = "/meteo/trend/" + String(md.trend_icon) + "_40.raw";
  drawPic(ICON_TREND_x, ICON_TREND_y, ICON_TREND_width, ICON_TREND_height, full_path_trend_icon);


  int dneskaDoW = board_time.getTime("%w").toInt();


  int zitraDoW = dneskaDoW + 1;
  if (zitraDoW > 6)
    zitraDoW = zitraDoW - 7;

  String dneska = String(day_of_week[dneskaDoW]);
  String zitra = String(day_of_week[zitraDoW]);


  show_text(280, 280, 80, ubuntu_regular_23, shown_today, dneska);
  show_text(400, 400, 80, ubuntu_regular_23, shown_tomorrow, zitra);
  shown_today = dneska;
  shown_tomorrow = zitra;

  // prekreslin pokazde
  drawPic(260, 105, 75, 75, "/meteo/cycle/cycle_" + String(md.clc_tdy) + "_40.raw");
  drawPic(370, 105, 75, 75, "/meteo/cycle/cycle_" + String(md.clc_tmr) + "_40.raw");

  //drawPic(260, 215, 50, 50, "/air1A.raw");
  drawPic(260, 225, 55, 53, "/meteo/air/air" + String(md.aqi) + "_55.raw");
  // d:\meteo\air\air1_55.raw

  String actual_hdo("");
  if (md.hdo1 < 0)
    actual_hdo = "??, ";
  else if (md.hdo1 == 0)
    actual_hdo = "now, ";
  else
    actual_hdo = "+" + String(md.hdo1) + "h, ";

  if (md.hdo2 < 0)
    actual_hdo = actual_hdo + "??";
  else if (md.hdo2 == 0)
    actual_hdo = actual_hdo + "now, ";
  else
    actual_hdo = actual_hdo + "+" + String(md.hdo2) + "h";

  show_text(330, 330, 236, ubuntu_regular_30, shown_hdo, actual_hdo);
  shown_hdo = actual_hdo;

  //Serial.println(shown_oat);
  //Serial.print("Sunrise:");
  //Serial.println(shown_sunrise);
  //Serial.print("Sunset:");
  //Serial.println(shown_sunset);
  //Serial.println(full_path_trend_icon);

  return;
}

void kresli_info_ctverecek(int vysledek_behu) {
  // kreslim info ctverecek
  if (vysledek_behu == 0) {
    //do nothing ... ale musi to tu byt abych neskoncil s 'jinou' chybou
  } else if (vysledek_behu == 1)
    drawBox(470, 310, 10, 10, TFT_MAGENTA);  //necham varovani, ze posledni aktualizace nebyla ok
  else if (vysledek_behu == 2)
    drawBox(470, 310, 10, 10, TFT_RED);  //necham varovani, ze wifi nebylo ok
  else
    drawBox(470, 310, 10, 10, TFT_BLACK);  //snad nemuze nastatat
}
