#include "Wireless.h"

volatile bool wifi_connected = false;

static const char* s_ssid = nullptr;
static const char* s_pass = nullptr;

static void on_wifi_event(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      wifi_connected = true;
      Serial.printf("[WiFi] connected, IP=%s RSSI=%d dBm\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      if (wifi_connected) {
        Serial.println("[WiFi] disconnected, retrying...");
      }
      wifi_connected = false;
      if (s_ssid && s_pass) {
        WiFi.begin(s_ssid, s_pass);
      }
      break;
    default:
      break;
  }
}

void WiFi_Connect_STA(const char* ssid, const char* pass) {
  s_ssid = ssid;
  s_pass = pass;
  WiFi.onEvent(on_wifi_event);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(ssid, pass);
  Serial.printf("[WiFi] begin SSID=\"%s\"\n", ssid);
}

String WiFi_LocalIP() {
  if (!wifi_connected) return String("0.0.0.0");
  return WiFi.localIP().toString();
}
