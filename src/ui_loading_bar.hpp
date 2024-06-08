#pragma once

#include "Arduino_GFX_Library.h"
#include <Arduino.h>
#include <string>

class ui_loading_bar {
private:
  Arduino_GFX* _gfx;

  uint16_t _x = 0;
  uint16_t _y = 0;
  int16_t _w = 0;
  int16_t _h = 0;

  uint16_t _percentage_for_1_bar = 0;
  uint16_t _bars_at_100_percent = 0;

  uint8_t _percentage = 0;

  void flush() {
    if (_percentage_for_1_bar == 0) {
      int16_t x_ = 0;
      int16_t y_ = 0;
      uint16_t w_ = 0;
      uint16_t h_ = 0;
      _gfx->getTextBounds("=", 0, 0, &x_, &y_, &w_, &h_);

      int16_t free_width = _w - (w_ * 2);
      _bars_at_100_percent = free_width / w_;
      _percentage_for_1_bar = 100 / _bars_at_100_percent;
    }

    uint8_t bars_count = std::min(_percentage, (uint8_t)100) / _percentage_for_1_bar;

    std::string bars;
    bars.reserve(bars_count);
    for (uint8_t i = 0; i < _bars_at_100_percent; i++) {
      bars += (i < bars_count) ? "=" : " ";
    }

    _gfx->setCursor(_x, _y);
    _gfx->printf("[%s]", bars.data());
  }

public:
  ui_loading_bar(Arduino_GFX* gfx, uint16_t x, uint16_t y, int16_t w, int16_t h) {
    _gfx = gfx;

    _x = x;
    _y = y;
    _w = w;
    _h = h;
  }

  void show() {
    _percentage = 0;
    flush();
  }

  void hide() {
    _gfx->fillRect(_x, _y, _w, _h, BLACK);
  }

  void updateByPercentage(uint8_t percentage) {
    _percentage = percentage;
    flush();
  }

  void updateByValue(uint32_t val, uint32_t max) {
    _percentage = std::floor((100 / (max / (double)std::max(val, (uint32_t)1))));
    flush();
  }
};
