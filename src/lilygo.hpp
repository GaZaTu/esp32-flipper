#pragma once

#include "pin_config.hpp"
#include <Arduino.h>
#include <esp_adc_cal.h>

void enableDisplay(bool enable = true) {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, enable ? HIGH : LOW);

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, enable ? HIGH : LOW);
}

uint32_t getBatteryVoltage() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t vbat = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; // The partial pressure is one-half
  return vbat;
}

// int getBatteryPercentage(uint32_t vbat = -1) {
//   const float min_voltage = 3.04;
//   const float max_voltage = 4.26;
//   const uint8_t scaled_voltage[] = {
//     254, 242, 230, 227, 223, 219, 215, 213, 210, 207,
//     206, 202, 202, 200, 200, 199, 198, 198, 196, 196,
//     195, 195, 194, 192, 191, 188, 187, 185, 185, 185,
//     183, 182, 180, 179, 178, 175, 175, 174, 172, 171,
//     170, 169, 168, 166, 166, 165, 165, 164, 161, 161,
//     159, 158, 158, 157, 156, 155, 151, 148, 147, 145,
//     143, 142, 140, 140, 136, 132, 130, 130, 129, 126,
//     125, 124, 121, 120, 118, 116, 115, 114, 112, 112,
//     110, 110, 108, 106, 106, 104, 102, 101, 99, 97,
//     94, 90, 81, 80, 76, 73, 66, 52, 32, 7,
//   };

//   if (vbat == -1) {
//     vbat = getBatteryVoltage();
//   }
//   for (int n = 0; n < sizeof(scaled_voltage); n++) {
//     float step = (max_voltage - min_voltage) / 256;
//     if (vbat > min_voltage + (step * scaled_voltage[n])) {
//       return 100 - n;
//     }
//   }
//   return 0;
// }
