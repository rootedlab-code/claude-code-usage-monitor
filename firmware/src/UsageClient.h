#pragma once
#include <Arduino.h>

#define MAX_MODELS 6

struct ModelStat {
  char     name[40];
  float    cost_usd;
  uint64_t tokens_in;
  uint64_t tokens_out;
};

struct UsageData {
  bool      online;            // ultima poll riuscita?
  uint32_t  last_update_ms;    // millis() all'ultimo success
  uint32_t  fetch_count;       // numero di poll riusciti (per anim "live")

  float    today_cost_usd;
  uint64_t today_tokens_in;
  uint64_t today_tokens_out;
  uint64_t today_cache_read;

  float    month_cost_usd;
  uint64_t month_tokens_in;
  uint64_t month_tokens_out;
  uint64_t month_cache_read;

  float    budget_monthly_usd;

  // Finestra 5h corrente (Claude Code rate-limit window)
  bool     win_active;
  uint32_t win_messages;
  float    win_cost_usd;
  uint64_t win_tokens_in;
  uint64_t win_tokens_out;
  uint16_t win_elapsed_min;
  uint16_t win_remaining_min;
  float    win_limit_usd;       // limite del piano (es. $200 per Max 5x)
  uint16_t win_limit_pct;       // 0-100+ (può sforare 100 se sopra limite)

  // Ultimi 7 giorni: index 0 = 6 giorni fa, index 6 = oggi
  float    last7_cost[7];
  char     last7_label[7][4];   // "10", "11", ...

  ModelStat models[MAX_MODELS];
  uint8_t   n_models;
};

// Avvia un task FreeRTOS dedicato che fa polling del bridge.
// host: es. "192.168.1.42", port: 8787, interval_ms: cadenza poll
// token: bearer token bridge (stringa vuota => header omesso, compatibilità v0.1)
void UsageClient_Begin(const char* host, uint16_t port, uint32_t interval_ms,
                       const char* token = "");

// Copia atomicamente l'ultimo snapshot in out. Sicuro da chiamare dal thread UI.
void UsageClient_Snapshot(UsageData& out);
