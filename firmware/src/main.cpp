/******************************************************************************
 *  Claude Code Usage Monitor — firmware Waveshare ESP32-S3-LCD-1.47
 *
 *  Si connette al WiFi locale, fa polling di un bridge Python sul PC
 *  (vedi ../bridge/) ogni N secondi e mostra in tempo reale costo + token
 *  + grafico 7 giorni + breakdown per modello su display ST7789 172x320.
 ******************************************************************************/
#include <Arduino.h>
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "RGB_lamp.h"
#include "Wireless.h"
#include "UsageClient.h"
#include "UsageUI.h"
#include "secrets.h"

static uint32_t last_ui_update_ms = 0;
static uint32_t last_ip_refresh_ms = 0;
static uint32_t last_rgb_count = 0;
static uint32_t fetch_pulse_until = 0;

static void rgb_set_status(bool online, bool pulse) {
  if (pulse) {
    Set_Color(60, 0, 120);   // viola: fetch appena riuscito
  } else if (online) {
    Set_Color(0, 20, 0);     // verde tenue
  } else {
    Set_Color(30, 0, 0);     // rosso tenue
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== Claude Code Usage Monitor ===");

  LCD_Init();
  Set_Backlight(80);

  Lvgl_Init();
  UsageUI_Init();

  WiFi_Connect_STA(WIFI_SSID, WIFI_PASS);

  UsageClient_Begin(BRIDGE_HOST, BRIDGE_PORT, POLL_INTERVAL_MS);

  Set_Color(0, 0, 30);  // blu tenue durante boot
}

void loop() {
  Timer_Loop();  // LVGL

  uint32_t now = millis();

  // Aggiorna IP nella status bar ogni 2 secondi
  if (now - last_ip_refresh_ms > 2000) {
    last_ip_refresh_ms = now;
    UsageUI_SetIp(WiFi_LocalIP().c_str());
  }

  // Aggiorna UI con snapshot ogni 400 ms
  if (now - last_ui_update_ms > 400) {
    last_ui_update_ms = now;
    UsageData d;
    UsageClient_Snapshot(d);
    UsageUI_Update(d);

    // RGB feedback: pulse 300ms ad ogni nuovo fetch
    if (d.fetch_count != last_rgb_count) {
      last_rgb_count = d.fetch_count;
      fetch_pulse_until = now + 300;
    }
    rgb_set_status(d.online, now < fetch_pulse_until);
  }

  delay(5);
}
