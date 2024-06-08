#pragma once

#include "Arduino_GFX_Library.h"
#include <Arduino.h>
#include <string>

class ui_label {
private:
  Arduino_GFX* _gfx;

  uint16_t _x = 0;
  uint16_t _y = 0;
  uint16_t _w = 0;
  uint16_t _h = 0;

  std::string _text;

  void flush() {
    int16_t x_ = 0;
    int16_t y_ = 0;
    _gfx->getTextBounds(_text.data(), 0, 0, &x_, &y_, &_w, &_h);

    _gfx->setCursor(_x, _y);
    _gfx->print(_text.data());
  }

public:
  ui_label(Arduino_GFX* gfx, uint16_t x, uint16_t y) {
    _gfx = gfx;

    _x = x;
    _y = y;
  }

  void show() {
    flush();
  }

  void hide() {
    _gfx->fillRect(_x, _y, _w, _h, BLACK);
  }

  void printf(const char* format, ...) {
    char line[1024];

    va_list args;
    va_start(args, format);
    size_t length = vsprintf(line, format, args);
    va_end(args);

    _text = std::string{line, length};

    flush();
  }
};
