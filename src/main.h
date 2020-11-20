#ifndef __INC_MAIN_H
#define __INC_MAIN_H

#include <stdint.h>

#define LEDS_PER_SEGMENT 7
#define MATRIX_WIDTH 18
#define MATRIX_HEIGHT 17

struct POINT {
  uint8_t x;
  uint8_t y;
};

typedef enum {
  HORIZONTAL = 0,
  VERTICAL = 1
} Orientation;

typedef enum {
  POS = 0,
  NEG = 1
} Direction;

struct SEGMENT {
  POINT start;
  uint8_t length;
  Orientation orientation;
  Direction direction;
};

struct PEN_CTX {
  POINT p;
  uint8_t segment;
  uint8_t pos;
  uint8_t frame;
};

//      3
//    .----.
// 2  |    | 4
//    >-6--<
// 1  |    | 5
//    `----'
//       0

byte s_DigitSegments[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 0, 0, 0, 1, 1, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 0, 0, 1, 1, 1, 1}, // 3
  {0, 0, 1, 0, 1, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 1, 1, 1, 0, 1, 1}, // 6
  {0, 0, 0, 1, 1, 1, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {0, 0, 1, 1, 1, 1, 1}  // 9
};

SEGMENT s_SegmentInfo[7] = {
  { { 7,  0 }, 7, HORIZONTAL, NEG }, // 0
  { { 0,  1 }, 7, VERTICAL,   POS }, // 1
  { { 0,  9 }, 7, VERTICAL,   POS }, // 2
  { { 1, 16 }, 7, HORIZONTAL, POS }, // 3
  { { 8, 15 }, 7, VERTICAL,   NEG }, // 4
  { { 8,  7 }, 7, VERTICAL,   NEG }, // 5
  { { 7,  8 }, 7, HORIZONTAL, NEG }  // 6
};

class Pen {
  public:
    virtual CRGB get(const PEN_CTX) = 0;
};

#endif
