#ifndef METEO_H
#define METEO_H

#include "config.h"

struct meteo_data {
  bool valid;
  int oat;
  char sunrise[6];
  char sunset[6];
  int sunlight;

  char w_icon[4];
  char trend_icon[20];
  int trend_temp;

  int clc_tdy;
  int clc_tmr;

  char aqi[5];

  int hdo1;
  int hdo2;
};

meteo_data update_meteo();
void update_all_elements(meteo_data md, int vsechno_prepis);
void kresli_info_ctverecek(int vysledek_behu);

#endif