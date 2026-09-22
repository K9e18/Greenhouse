#pragma once

#include <Arduino.h>

// Custom degree symbol (°)
const byte degree_Symbol[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Custom "fan is on" symbol
const byte fan_is_on_Symbol[8] = {
  0b00100,
  0b10101,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

// Custom "water drop" symbol
const byte water_drop_Symbol[8] = {
      0b00100,
      0b00100,
      0b01110,
      0b01110,
      0b11111,
      0b11111,
      0b11111,
      0b01110
}
