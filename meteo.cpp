#include "meteo.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>
#include <ESP32Time.h>



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

    time_t sunrise = w_doc["weather"]["sunrise"];
    struct tm sunrise_local = *localtime(&sunrise);
    strftime(md.sunrise, sizeof(md.sunrise), "%H:%M", &sunrise_local);

    // odstran nulu na pocatku vychodu slunce. if je zbytecne, ale radeji jo
    if (md.sunrise[0] == '0')
      memmove(md.sunrise, md.sunrise + 1, strlen(md.sunrise));

    time_t sunset = w_doc["weather"]["sunset"];
    struct tm sunset_local = *localtime(&sunset);
    strftime(md.sunset, sizeof(md.sunset), "%H:%M", &sunset_local);

    // decide if its day or night
    ESP32Time board_time(0);
    unsigned long current_epoch = board_time.getEpoch();
    if ((current_epoch > w_doc["weather"]["sunrise"]) && (current_epoch < w_doc["weather"]["sunset"]))
      md.sunlight = 1;
    else
      md.sunlight = 0;


    md.valid = true;
  }

  Serial.println("meteo updated");
  return md;
}