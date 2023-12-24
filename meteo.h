#ifndef METEO_H
#define METEO_H

#include "config.h"

struct meteo_data{
  bool valid;
  int oat;
  char sunrise[6];
  char sunset[6];
};

meteo_data update_meteo();

#endif