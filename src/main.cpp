#include <Arduino.h>
#include <FastLED.h>
#include "RTClib.h"
#include "freeram.h"
#include "main.h"

// #define TEST_DAYS_LEFT

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
      // Don't animate during fade
      PEN_CTX tmp = {ctx.p, ctx.segment, ctx.pos, -1, ctx.ledIdx };
      CRGB from = m_fromPen->get(tmp);
      CRGB to = m_toPen->get(tmp);
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
  Direction direction;
};

class SparklePen : public Pen {
  public:
    SparklePen(Pen* pen, CRGB color, SPARKLE* sparkles) : m_pen(pen), m_color(color), m_sparkles(sparkles), nextSparkle(0) {}
    CRGB get(const PEN_CTX ctx) {
      bool shouldAnimate = ctx.frame > 0;
      if (shouldAnimate) {
        if (ctx.frame == 0 && ctx.pos == 0 && ctx.ledIdx == 0) {
          nextSparkle = 0;
        }

        if (nextSparkle < 10) {
          if (random(50) == 0) {
            SPARKLE *s = &m_sparkles[nextSparkle];
            s->p = { ctx.p.x, ctx.p.y };
            s->amount = random(16) * 16;
            s->amount = 0;
            s->direction = Direction(random(2));
            nextSparkle++;
          }
        }
      }

      for (uint8_t i = 0; i < nextSparkle; i++) {
        SPARKLE *s = &m_sparkles[i];
        if (s->p.x == ctx.p.x && s->p.y == ctx.p.y) {
          CRGB c = blend(m_pen->get(ctx), m_color, s->amount);
          if (shouldAnimate) {
            if (s->direction == POS) {
              s->amount += s->amount == 240 ? 15 : 16;
              if (s->amount >= 255) {
                s->direction = NEG;
              }
            } else {
              s->amount -= s->amount == 255 ? 15 : 16;
              if (s->amount == 0) {
                s->direction = POS;
              }
            }
          }
          return c;
        }
      }
      return m_pen->get(ctx);
    }
  private:
    Pen *m_pen;
    uint8_t nextSparkle;
    CRGB m_color;
    SPARKLE* m_sparkles;
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
        PEN_CTX ctx = {flipped, i, pos, frame, ledIdx};
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

GoldenGateBridgePen goldenGateBridgePen;

FaderPen faderPen;

SPARKLE sparkles0[49];
SPARKLE sparkles1[49];

SolidPen greenkPen(SolidPen(CRGB::Green));
SparklePen xmasSparklePen(&greenkPen, CRGB::Red, sparkles0);

CRGB c5[] = {CRGB::Blue, CRGB::White};
SegmentCyclePen hanukkahPen(SegmentCyclePen(c5, 2));
SparklePen hanukkahSparklePen(&hanukkahPen, CRGB::Yellow, sparkles0);

CRGB c6[] = {CRGB::Red, CRGB::Green};
SegmentCyclePen kwanzaaPen(SegmentCyclePen(c6, 2));
SparklePen kwanzaaSparklePen(&kwanzaaPen, CRGB::White, sparkles0);

CRGB c7[] = {CRGB::Red, CRGB::White, CRGB::Blue};
SegmentCyclePen victoryPen(SegmentCyclePen(c7, 3));
SparklePen victorySparklePen0(&victoryPen, CRGB::White, sparkles0);
SparklePen victorySparklePen1(&victoryPen, CRGB::White, sparkles1);

Pen* pens[] = {
  &prideFlagPen, &panAfricanPen, &patrioticPen, &democratPen, &californiaPen, &transFlagPen, &giantsPen, &pinkPen
};
// Pen* pens[] = {
//   &prideFlagPen, &giantsSparklePen
// };
uint8_t numPens = sizeof(pens) / sizeof(*pens);

CRGB leds0[49];
CRGB leds1[49];
CLEDController *controllers[2];
RTC_DS3231 rtc;

DateTime inauguration(DateTime(2021, 1, 20, 0, 0, 0));
Pen* current;
Pen* next;


uint8_t _getDaysLeft(DateTime now) {
  TimeSpan span = inauguration - now;
  int8_t left = span.days() + (span.hours() > 0 || span.seconds() > 0 ? 1 : 0);
  return left > 0 ? left : 0;
}

uint8_t getDaysLeft() {
  return _getDaysLeft(rtc.now());
}

#ifdef TEST_DAYS_LEFT
void testDaysLeft() {
  DateTime now = rtc.now();
  Serial.print("Current time is ");
  Serial.println(now.timestamp());

  Serial.print("Days left ");
  Serial.println(_getDaysLeft(now));

  DateTime hourBeforeInauguration(DateTime(2021, 1, 19, 23, 0, 0));
  Serial.print("hourBeforeInauguration ");
  Serial.println(_getDaysLeft(hourBeforeInauguration));

  Serial.print("inauguration ");
  Serial.println(_getDaysLeft(inauguration));

  DateTime hourAfterInauguration(DateTime(2021, 1, 20, 1, 0, 0));
  Serial.print("hourAfterInauguration ");
  Serial.println(_getDaysLeft(hourAfterInauguration));

  DateTime dayAfterInauguration(DateTime(2021, 1, 21, 1, 0, 0));
  Serial.print("dayAfterInauguration ");
  Serial.println(_getDaysLeft(dayAfterInauguration));

  DateTime hourBeforeNewYear(DateTime(2020, 12, 31, 23, 0, 0));
  Serial.print("hourBeforeNewYear ");
  Serial.println(_getDaysLeft(hourBeforeNewYear));

  DateTime onNewYear(DateTime(2021, 1, 1, 0, 0, 0));
  Serial.print("onNewYear ");
  Serial.println(_getDaysLeft(onNewYear));

  DateTime hourAfterNewYear(DateTime(2021, 1, 1, 1, 0, 0));
  Serial.print("hourAfterNewYear ");
  Serial.println(_getDaysLeft(hourAfterNewYear));

  Serial.flush(); 
}
#endif 

void setDate() {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

Pen* getNextPen(int8_t daysLeft, Pen* current) {
  if (daysLeft == 0) {
    if (current == &victorySparklePen0) {
      return &victorySparklePen1;
    } else {
      return &victorySparklePen0;
    }
  }

  //DateTime now(DateTime(2020, 12, 28, 0, 0, 0));
  //DateTime now(DateTime(2020, 12, 12, 0, 0, 0));
  //DateTime now(DateTime(2020, 12, 22, 0, 0, 0));
  DateTime now = rtc.now();

  Pen* holiday = NULL;
  DateTime chirstmasStart(DateTime(2020, 12, 18, 0, 0, 0));
  DateTime chirstmasEnd(DateTime(2020, 12, 26, 0, 0, 0));
  if (now >= chirstmasStart && now <= chirstmasEnd) {
    holiday = &xmasSparklePen;
  }

  DateTime hanukkahStart(DateTime(2020, 12, 10, 0, 0, 0));
  DateTime hanukkahEnd(DateTime(2020, 12, 18, 0, 0, 0));
  if (now >= hanukkahStart && now <= hanukkahEnd) {
    holiday = &hanukkahSparklePen;
  }

  DateTime kwanzaaStart(DateTime(2020, 12, 26, 0, 0, 0));
  DateTime kwanzaaEnd(DateTime(2021, 1, 1, 0, 0, 0));
  if (now >= kwanzaaStart && now <= kwanzaaEnd) {
    holiday = &kwanzaaSparklePen;
  }

  uint8_t numPensWithHoliday = holiday == NULL ? numPens : numPens + 1; 

  Pen *next;
  do {
    long r = random(numPensWithHoliday);
    if (r == numPens) {
      next = holiday;
    } else {
     next = pens[r];
    }
  } while (next == current);

  return next;
}

void setup() {
  Serial.begin(57600);
  while (!Serial);

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  //setDate();

  Serial.println(F("Starting countdown..."));
  Serial.flush();
  getFreeRam();

  controllers[0] = &FastLED.addLeds<WS2812, 2, GRB>(leds0, LEDS_PER_SEGMENT * 7).setCorrection(TypicalSMD5050);;
  controllers[1] = &FastLED.addLeds<WS2812, 3, GRB>(leds1, LEDS_PER_SEGMENT * 7).setCorrection(TypicalSMD5050);;
  FastLED.show();

  randomSeed(analogRead(0));
  current = getNextPen(getDaysLeft(), NULL);
}

void draw(uint8_t daysLeft, uint8_t frame, Pen* pen) {
  uint8_t pos_0 = daysLeft / 10;
  uint8_t pos_1 = daysLeft % 10;
  drawDigit(pos_0, 0, frame, pen, leds0);
  drawDigit(pos_1, 1, frame, pen, leds1);
  FastLED.show();
}

void loop() {
  getFreeRam();
  Serial.flush();

  DateTime now = rtc.now();

#ifdef TEST_DAYS_LEFT
  testDaysLeft();
  delay(1000000);
  return;
#endif

  uint32_t ts = now.unixtime();

  int8_t daysLeft = getDaysLeft();

  int32_t frame = 0;
  while (rtc.now().unixtime() - ts < 120) {
    draw(daysLeft, frame, current);
    frame++;
    delay(100); 
  }

  next = getNextPen(daysLeft, current);

  faderPen.setFromPen(current);
  faderPen.setToPen(next);

  for (int32_t frame = 0; frame < 256; frame++) {
    draw(daysLeft, frame, &faderPen);
    delay(10); 
  }

  current = next;
}
