#include "hourly_temp_screen.h"
#include "config.h"
#include "show_elements.h"
#include "ubuntu_fonts.h"
#include "InterpolationLib.h"

#include <ESP32Time.h>

extern TTGOClass* ttgo;

void show_hourly_temp_screen() {
  Serial.println("showing hourly temperature screen");

  ttgo->tft->fillScreen(TFT_WHITE);
  //ttgo->tft->drawLine(10, 10, 400, 300, TFT_RED);

  const int numValues = 10;
  double xCasy[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  double yTeploty[10] = { 15, 17, 21, 20, 23, 18, 17, 15, 14, 15 };



  for (int ix = 0; ix < numValues - 1; ix++) {
    int xCas1 = xCasy[ix] * 40;
    int xCas2 = xCasy[ix + 1] * 40;

   // ttgo->tft->drawLine(xCas1, 15 * (yTeploty[ix] - 10), xCas2, 15 * (yTeploty[ix + 1] - 10), TFT_BLUE);
  }


  for (double f_temp = 0.01; f_temp < 10.0; f_temp += 0.01) {

    double xs1 = (f_temp - 0.01) * 40.0;
    double xs2 = (f_temp)*40.0;

    //Serial.println(xs1);

    double ys1 = Interpolation::CatmullSpline(xCasy, yTeploty, numValues, f_temp - 0.01);
    double ys2 = Interpolation::CatmullSpline(xCasy, yTeploty, numValues, f_temp);

    ttgo->tft->drawLine(xs1, 15 * (ys1 - 10), xs2, 15 * (ys2 - 10), TFT_RED);
    ttgo->tft->drawLine(xs1, 15 * (ys1 - 10) - 1, xs2, 15 * (ys2 - 10) - 1, TFT_RED);
    ttgo->tft->drawLine(xs1, 15 * (ys1 - 10) + 1, xs2, 15 * (ys2 - 10) + 1, TFT_RED);
    ttgo->tft->drawLine(xs1, 15 * (ys1 - 10) - 2, xs2, 15 * (ys2 - 10) - 2, TFT_RED);
    ttgo->tft->drawLine(xs1, 15 * (ys1 - 10) + 2, xs2, 15 * (ys2 - 10) + 2, TFT_RED);
  }





  /// cekam na konec
  delay(1000);  // abych stihl dat prst pryc
  ESP32Time board_time(0);
  unsigned long current_epoch = board_time.getEpoch();
  unsigned long expected_end_epoch = current_epoch + 30;  // za 30s
  String shown_counter("XX");
  String actual_counter("XX");

  while ((expected_end_epoch > current_epoch) && (ttgo->touched() == 0)) {
    actual_counter = (String)(expected_end_epoch - current_epoch);
    show_text(450, 450, 293, ubuntu_regular_23, shown_counter, actual_counter);
    shown_counter = actual_counter;
    current_epoch = board_time.getEpoch();
    delay(100);
  }
  ttgo->tft->fillScreen(TFT_WHITE);

  return;
}