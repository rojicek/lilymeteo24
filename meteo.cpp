#include "meteo.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>
#include <ESP32Time.h>

#define DELKA_PROGRAMU 9000  // sekundy

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

    Serial.print("raw sunrise:");
    Serial.println(sunrise);

    // odstran nulu na pocatku vychodu slunce. if je zbytecne, ale radeji jo
    if (md.sunrise[0] == '0')
      memmove(md.sunrise, md.sunrise + 1, strlen(md.sunrise));

    time_t sunset = w_doc["weather"]["sunset"];
    struct tm sunset_local = *localtime(&sunset);
    strftime(md.sunset, sizeof(md.sunset), "%H:%M", &sunset_local);

    Serial.print("raw sunset:");
    Serial.println(sunset);

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
    md.hdo1 = 4;
    md.hdo2 = 6;

    JsonArray hdo_arr = w_doc["hdo_intervals"].as<JsonArray>();
    // uz mam current_epoch
    // smycka pro hodiny, ktere testuji

    //  Serial.println(hdo_arr[0][0]);
    //  Serial.print("*->");

    for (int hour = 0; hour < 12; hour++) {
      unsigned long one_start = current_epoch + hour * 3600;
      unsigned long one_end = DELKA_PROGRAMU + one_start;

      int ve_drahe_sazbe = 0;


      for (JsonVariant value : hdo_arr) {

        ve_drahe_sazbe += kolik_je_prunik(one_start, one_end, value[0].as<const int>(), value[1].as<const int>());
      }
      Serial.print("start v +");
      Serial.print(hour);
      Serial.print("h = ");
      Serial.print(ve_drahe_sazbe/60.0);
      Serial.println(" min");
    }

    Serial.println("<<<array");


    md.valid = true;
  }

  Serial.println("meteo updated");
  return md;
}
