#include "meteo.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Time.h>



meteo_data update_meteo() {
  meteo_data md;

  StaticJsonDocument<1000> w_doc;
  HTTPClient http;
  String url = "https://www.rojicek.cz/meteo/meteo-query.php?pwd=pa1e2";

  Serial.println("updating meteo 1");
  http.begin(url.c_str());
  Serial.println("updating meteo 2");

  int httpResponseCode = http.GET();
  Serial.println("updating meteo 3");

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

    if (md.sunrise[0] == '0')
      memmove(md.sunrise, md.sunrise + 1, strlen(md.sunrise));

    time_t sunset = w_doc["weather"]["sunset"];
    struct tm sunset_local = *localtime(&sunset);
    strftime(md.sunset, sizeof(md.sunset), "%H:%M", &sunset_local);


    md.valid = true;
  }


  return md;
}