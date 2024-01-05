#include "hourly_temp_screen.h"
#include "config.h"
#include "show_elements.h"
#include "ubuntu_fonts.h"

#include <ESP32Time.h>

extern TTGOClass* ttgo;

void show_hourly_temp_screen() {
  Serial.println("showing hourly temperature screen");

  ttgo->tft->fillScreen(TFT_WHITE);


  /// cekam na konec
  delay(1000); // abych stihl dat prst pryc
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