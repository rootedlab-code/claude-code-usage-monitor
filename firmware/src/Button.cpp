#include "Button.h"

#define BTN_PIN         0
#define DEBOUNCE_MS     50
#define LONG_THRESHOLD  500
#define VLONG_THRESHOLD 5000

static bool     s_last_raw = HIGH;       // pull-up: HIGH = rilasciato
static uint32_t s_stable_since = 0;
static bool     s_pressed = false;
static uint32_t s_press_start = 0;
static bool     s_vlong_fired = false;

void Button::begin() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  s_last_raw = digitalRead(BTN_PIN);
  s_stable_since = millis();
}

Button::Event Button::poll() {
  uint32_t now = millis();
  bool raw = digitalRead(BTN_PIN);

  if (raw != s_last_raw) {
    s_last_raw = raw;
    s_stable_since = now;
    return NONE;
  }
  if ((now - s_stable_since) < DEBOUNCE_MS) return NONE;

  bool pressed_now = (raw == LOW);

  // Transizione: rilasciato → premuto
  if (pressed_now && !s_pressed) {
    s_pressed = true;
    s_press_start = now;
    s_vlong_fired = false;
    return NONE;
  }

  // Transizione: premuto → rilasciato (TAP o LONG)
  if (!pressed_now && s_pressed) {
    s_pressed = false;
    uint32_t held = now - s_press_start;
    if (s_vlong_fired) return NONE;            // VERY_LONG già emesso
    if (held < LONG_THRESHOLD)   return TAP;
    if (held < VLONG_THRESHOLD)  return LONG;
    return VERY_LONG;                          // rilasciato dopo i 5s
  }

  // Ancora premuto: emetti VERY_LONG mid-press a 5s per feedback immediato
  if (pressed_now && s_pressed && !s_vlong_fired) {
    uint32_t held = now - s_press_start;
    if (held >= VLONG_THRESHOLD) {
      s_vlong_fired = true;
      return VERY_LONG;
    }
  }

  return NONE;
}
