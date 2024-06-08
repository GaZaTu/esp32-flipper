#pragma once

#include "Arduino_GFX_Library.h"
#include <Arduino.h>
#include <deque>
#include <string>

class ui_scrolling_text {
private:
  Arduino_GFX* _gfx;

  uint16_t _x = 0;
  uint16_t _y = 0;
  int16_t _w = 0;
  int16_t _h = 0;

  std::deque<std::string> _lines;

  uint16_t _line_limit = 0;
  uint16_t _line_height = 0;

  void flush() {
    if (_line_limit == 0) {
      int16_t x_ = 0;
      int16_t y_ = 0;
      uint16_t w_ = 0;
      _gfx->getTextBounds("ty", 0, 0, &x_, &y_, &w_, &_line_height);

      _line_height += 2;
      _line_limit = _h / _line_height;

      _gfx->drawRect(_x, _y , _w, _h, WHITE);
    }

    uint16_t y = _y + 4;
    for (std::string& line : _lines) {
      _gfx->fillRect(_x + 4, y, _w - 8, _line_height, BLACK);

      _gfx->setCursor(_x + 4, y);
      _gfx->print(line.data());

      y += _line_height;
    }
  }

public:
  ui_scrolling_text(Arduino_GFX* gfx, uint16_t x, uint16_t y, int16_t w, int16_t h) {
    _gfx = gfx;

    _x = x;
    _y = y;
    _w = w;
    _h = h;
  }

  size_t printf(const char* format, ...) {
    char line[1024];

    va_list args;
    va_start(args, format);
    size_t length = vsprintf(line, format, args);
    va_end(args);

    if (_lines.size() > 0 && _lines.size() == _line_limit) {
      _lines.pop_front();
    }

    _lines.push_back(std::string{line, length});

    flush();

    return length;
  }

  void printHex8(const uint8_t* data, uint8_t length, char const* separator = nullptr) {
    for (int i = 0; i < length; i++) {
      if (data[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(data[i], HEX);

      if (separator != nullptr) {
        Serial.print(" ");
      }
    }
  }
};
