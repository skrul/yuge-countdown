#include <Arduino.h>
#include <FastLED.h>
#include "RTClib.h"
#include "freeram.h"
#include "main.h"

class HorizStripesPen : public Pen {
  public:
    HorizStripesPen(CRGB *colors, int size) : m_colors(colors), m_size(size) {}

    CRGB get(const PEN_CTX ctx) {
      uint8_t y = ctx.p.y;
      if (m_size == 3) {
        if (y < 5) {
          return m_colors[0];
        } else {
          if (y < 12) {
          return m_colors[1];
          } else {
          return m_colors[2];
          }
        }
      } else {
        return m_colors[int(y * (m_size / float(MATRIX_HEIGHT)))];
      }

    }

    CRGB *m_colors;
    int m_size;
};

class VertStripesPen : public Pen {
  public:
    VertStripesPen(CRGB *colors, int size) : m_colors(colors), m_size(size) {}

    CRGB get(const PEN_CTX ctx) {
      return m_colors[int(ctx.p.x * (m_size / float(MATRIX_WIDTH)))];
    }

    CRGB *m_colors;
    int m_size;
};

class SolidPen : public Pen {
  public:
    SolidPen(CRGB color) : m_color(color) {}
    CRGB get(const PEN_CTX ctx) {
      return m_color;
    }
    CRGB m_color;
};

class SegmentCyclePen : public Pen {
  public:
    SegmentCyclePen(CRGB *colors, int size) : m_colors(colors), m_size(size) {}
    CRGB get(const PEN_CTX ctx) {
      return m_colors[(ctx.segment + ctx.pos) % m_size];
    }
  private:
    CRGB *m_colors;
    int m_size;
};

class GoldenGateBridgePen : public Pen {
  public:
    GoldenGateBridgePen() {}
    CRGB get(const PEN_CTX ctx) {
      if (ctx.segment == 3) {
        return 0x101010;
      } else {
        return 0xF04A00;
      }
    }
};

class FaderPen : public Pen {
  public:
    FaderPen() {}
    CRGB get(const PEN_CTX ctx) {
      CRGB from = m_fromPen->get(ctx);
      CRGB to = m_toPen->get(ctx);
      return blend(from, to, ctx.frame % 256);
    }
    void setFromPen(Pen* fromPen) {
      m_fromPen = fromPen;
    }
    void setToPen(Pen* toPen) {
      m_toPen = toPen;
    }
  private:
    Pen *m_fromPen;
    Pen *m_toPen;
};

struct SPARKLE {
  POINT p;
  fract8 amount;
};

class SparklePen : public Pen {
  public:
    SparklePen(Pen* pen) : m_pen(pen){}
    CRGB get(const PEN_CTX ctx) {
      if (ctx.frame == 0) {
        for (uint8_t i = 0; i < 10; i++) {
          SPARKLE s = sparkles[i];
          s.p.x = random(MATRIX_WIDTH * 2);
          s.p.y = random(MATRIX_HEIGHT);
          s.amount = random(256);
        }
      }
      return m_pen->get(ctx);
    }
  private:
    Pen *m_pen;
    SPARKLE sparkles[10];
};

void drawDigit(uint8_t num, uint8_t pos, uint8_t frame, Pen *pen, CRGB *leds) {
  uint8_t *segs = s_DigitSegments[num];
  uint8_t ledIdx = 0;
  for (uint8_t i = 0; i < 7; i++) {
    SEGMENT seg = s_SegmentInfo[i];
    POINT p = seg.start;
    p.x = p.x + (pos * 9);
    for (uint8_t j = 0; j < 7; j++) {
      int inc = seg.direction == POS ? 1 : -1;
      if (segs[i] == 1) {
        POINT flipped = p;
        flipped.y = MATRIX_HEIGHT - p.y - 1;
        PEN_CTX ctx = {flipped, i, pos, frame};
        leds[ledIdx] = pen->get(ctx);
      } else {
        leds[ledIdx] = CRGB::Black;
      }
      ledIdx++;
      if (seg.orientation == HORIZONTAL) {
        p.x += inc;
      } else {
        p.y += inc;
      }
    }
  }
}

CRGB c0[] = {CRGB::Red, CRGB::White, CRGB::Blue};
HorizStripesPen patrioticPen(HorizStripesPen(c0, 3));

SolidPen democratPen(SolidPen(CRGB::Blue));

SolidPen pinkPen(SolidPen(CRGB::DeepPink));

SolidPen giantsPen(SolidPen(0xF04A00));

CRGB c1[] = {CRGB::Blue, CRGB::Yellow};
SegmentCyclePen californiaPen(SegmentCyclePen(c1, 2));

CRGB c2[] = {0x55CDFC, 0xF7A8B8, CRGB::White, 0xF7A8B8, 0x55CDFC};
HorizStripesPen transFlagPen(HorizStripesPen(c2, 5));

CRGB c3[] = {0x830303, 0x080808, 0x12663b};
HorizStripesPen panAfricanPen(HorizStripesPen(c3, 3));

CRGB c4[] = {0xFF0018, 0xFFA52C, 0xFFFF41, 0x008018, 0x0000F9, 0x86007D};
HorizStripesPen prideFlagPen(HorizStripesPen(c4, 6));

CRGB c5[] = {0x169B62, CRGB::White, 0xFF8200};
VertStripesPen irelandPen(VertStripesPen(c3, 3));

CRGB c6[] = {0xaa0000, 0xb3995d};
SegmentCyclePen fortyNinersPen(SegmentCyclePen(c6, 2));

GoldenGateBridgePen goldenGateBridgePen;

FaderPen faderPen;

Pen* pens[] = {
  &prideFlagPen, &panAfricanPen, &patrioticPen, &democratPen, &californiaPen, &transFlagPen, &giantsPen, &pinkPen
};
// Pen* pens[] = {
//   &fortyNinersPen 
// };
uint8_t numPens = sizeof(pens) / sizeof(*pens);


CRGB leds0[49];
CRGB leds1[49];
CLEDController *controllers[2];
RTC_DS3231 rtc;

DateTime inauguration(DateTime(2021, 1, 20, 8, 0, 0));
Pen* current;
Pen* next;

Pen* getNextPen() {
  DateTime now = rtc.now();

}

void setup() {
  Serial.begin(57600);
  while (!Serial);

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  Serial.println(F("Starting countdown..."));
  Serial.flush();
  getFreeRam();

  controllers[0] = &FastLED.addLeds<WS2812, 2, GRB>(leds0, LEDS_PER_SEGMENT * 7).setCorrection(TypicalSMD5050);;
  controllers[1] = &FastLED.addLeds<WS2812, 3, GRB>(leds1, LEDS_PER_SEGMENT * 7).setCorrection(TypicalSMD5050);;
  FastLED.show();

  randomSeed(analogRead(0));
  current = pens[random(numPens)];
}

void loop() {
  getFreeRam();
  Serial.flush();

  //DateTime now(DateTime(2020, 11, 20, 8, 04, 00));
  DateTime now = rtc.now();
  TimeSpan span = inauguration - now;
  int8_t daysLeft = span.days() + (span.hours() > 0 || span.seconds() > 0 ? 1 : 0);
  Serial.println(daysLeft);
  Serial.flush();
  uint8_t pos_0 = 0;
  uint8_t pos_1 = 0;
  if (daysLeft > 0) {
    pos_0 = daysLeft / 10;
    pos_1 = daysLeft % 10;
  }

  for (int j = 0; j < 50; j++) {
    drawDigit(pos_0, 0, j, current, leds0);
    drawDigit(pos_1, 1, j, current, leds1);
    FastLED.show();
    delay(100); 
  }

  do {
    next = pens[random(numPens)];
  } while (next == current);

  faderPen.setFromPen(current);
  faderPen.setToPen(next);

  for (int j = 0; j < 256; j++) {
    drawDigit(pos_0, 0, j, &faderPen, leds0);
    drawDigit(pos_1, 1, j, &faderPen, leds1);
    FastLED.show();
    delay(10); 
  }

  current = next;
}
