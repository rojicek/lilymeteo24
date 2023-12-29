#include "meteo.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>
#include <ESP32Time.h>

#define DELKA_PROGRAMU 9000  // sekundy
#define RANNI_HDO_HODINA 9   // druhe hdo chci co nejblize tolika hodin rano (konec programu)

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
    String payload = http.getString();
    DeserializationError error = deserializeJson(w_doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      md.valid = false;
      return md;
    }

    md.oat = w_doc["weather"]["temp"];
    //md.oat = random(-11,14); // testovani

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
    time_t ranni_hdo_seconds = current_epoch + 86400;

    memcpy(&ranni_hdo, localtime(&ranni_hdo_seconds), sizeof(struct tm));
    ranni_hdo.tm_hour = RANNI_HDO_HODINA;
    ranni_hdo.tm_min = 0;
    ranni_hdo.tm_sec = 0;

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
  }

  Serial.println("meteo updated");
  return md;
}
