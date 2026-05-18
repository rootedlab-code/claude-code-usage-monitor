#pragma once
#include <Arduino.h>

// Pulsante BOOT (GPIO 0) — polled, debounced.
//
// NB: GPIO0 è anche il strap pin di download mode dell'ESP32-S3, quindi NON
// usiamo interrupt. Il polling è OK perché chiamiamo Button::poll() ad alta
// frequenza dal main loop.
namespace Button {

enum Event {
  NONE,
  TAP,         // release entro 500 ms
  LONG,        // release tra 500 e 5000 ms
  VERY_LONG    // ancora premuto a 5000 ms (fire mid-press per feedback)
};

void begin();

// Da chiamare ad ogni iterazione del loop. Restituisce un evento al massimo
// per chiamata (consumato e azzerato).
Event poll();

} // namespace Button
