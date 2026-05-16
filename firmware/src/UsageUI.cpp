#include "UsageUI.h"
#include "Display_ST7789.h"
#include <stdio.h>
#include <string.h>

// ----- Font: usa quelli opzionali se abilitati in lv_conf.h, altrimenti fallback -----
#if LV_FONT_MONTSERRAT_32
  #define FONT_HUGE   &lv_font_montserrat_32
#elif LV_FONT_MONTSERRAT_28
  #define FONT_HUGE   &lv_font_montserrat_28
#elif LV_FONT_MONTSERRAT_24
  #define FONT_HUGE   &lv_font_montserrat_24
#else
  #define FONT_HUGE   &lv_font_montserrat_16
#endif

#if LV_FONT_MONTSERRAT_22
  #define FONT_BIG    &lv_font_montserrat_22
#elif LV_FONT_MONTSERRAT_20
  #define FONT_BIG    &lv_font_montserrat_20
#elif LV_FONT_MONTSERRAT_18
  #define FONT_BIG    &lv_font_montserrat_18
#else
  #define FONT_BIG    &lv_font_montserrat_16
#endif

#define FONT_MED    &lv_font_montserrat_16
#define FONT_SMALL  &lv_font_montserrat_14
#define FONT_TINY   &lv_font_montserrat_12

#define STATUS_H 18
#define SCREEN_W LCD_WIDTH    // 172
#define SCREEN_H LCD_HEIGHT   // 320
#define PANEL_W  SCREEN_W
#define PANEL_H  (SCREEN_H - STATUS_H)

#define ROTATE_INTERVAL_MS 6000

// ----- Stato widget -----
static lv_obj_t* status_dot;
static lv_obj_t* status_label;
static lv_obj_t* status_ip;

static lv_obj_t* panels[4];

// Tab Costo
static lv_obj_t* today_cost_lbl;
static lv_obj_t* month_cost_lbl;
static lv_obj_t* updated_lbl;

// Tab Finestra 5h
static lv_obj_t* win_msg_lbl;
static lv_obj_t* win_cost_lbl;
static lv_obj_t* win_tokens_lbl;
static lv_obj_t* win_elapsed_bar;
static lv_obj_t* win_reset_lbl;

// Tab Grafico
static lv_obj_t* chart;
static lv_chart_series_t* chart_series;
static lv_obj_t* chart_day_lbls[7];
static lv_obj_t* chart_max_lbl;

// Tab Modelli
#define MODEL_ROWS 5
static lv_obj_t* model_row[MODEL_ROWS];
static lv_obj_t* model_name_lbl[MODEL_ROWS];
static lv_obj_t* model_cost_lbl[MODEL_ROWS];
static lv_obj_t* model_bar[MODEL_ROWS];
static lv_obj_t* model_empty_lbl;

static uint8_t active_panel = 0;

// ----- Helper di formattazione -----
static void fmt_money(char* dst, size_t cap, float v) {
  if (v >= 1000.0f) snprintf(dst, cap, "$%.0f", v);
  else              snprintf(dst, cap, "$%.2f", v);
}

static void fmt_tokens(char* dst, size_t cap, uint64_t v) {
  if (v < 1000ULL)              snprintf(dst, cap, "%llu", (unsigned long long)v);
  else if (v < 1000000ULL)      snprintf(dst, cap, "%.1fK", v / 1000.0);
  else if (v < 1000000000ULL)   snprintf(dst, cap, "%.2fM", v / 1000000.0);
  else                          snprintf(dst, cap, "%.2fB", v / 1000000000.0);
}

// estrae sigla modello: "claude-opus-4-7" -> "Opus 4.7"
static void short_model_name(const char* full, char* dst, size_t cap) {
  if (!full || !*full) { snprintf(dst, cap, "?"); return; }
  // cerca opus/sonnet/haiku
  const char* fam = nullptr;
  if (strstr(full, "opus"))   fam = "Opus";
  else if (strstr(full, "sonnet")) fam = "Sonnet";
  else if (strstr(full, "haiku"))  fam = "Haiku";
  else { snprintf(dst, cap, "%s", full); return; }

  // estrai prima coppia di numeri "4-7" -> "4.7"
  int maj = 0, min = -1;
  const char* p = full;
  while (*p) {
    if (*p >= '0' && *p <= '9') {
      maj = atoi(p);
      while (*p >= '0' && *p <= '9') p++;
      if (*p == '-' && p[1] >= '0' && p[1] <= '9') {
        min = atoi(p + 1);
      }
      break;
    }
    p++;
  }
  if (min >= 0) snprintf(dst, cap, "%s %d.%d", fam, maj, min);
  else if (maj > 0) snprintf(dst, cap, "%s %d", fam, maj);
  else snprintf(dst, cap, "%s", fam);
}

// ----- Costruzione UI -----

static void make_status_bar(lv_obj_t* parent) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_remove_style_all(bar);
  lv_obj_set_size(bar, SCREEN_W, STATUS_H);
  lv_obj_set_pos(bar, 0, 0);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x101820), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

  status_dot = lv_obj_create(bar);
  lv_obj_remove_style_all(status_dot);
  lv_obj_set_size(status_dot, 8, 8);
  lv_obj_set_pos(status_dot, 5, 5);
  lv_obj_set_style_radius(status_dot, 4, LV_PART_MAIN);
  lv_obj_set_style_bg_color(status_dot, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(status_dot, LV_OPA_COVER, LV_PART_MAIN);

  status_label = lv_label_create(bar);
  lv_label_set_text(status_label, "...");
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xeeeeee), LV_PART_MAIN);
  lv_obj_set_style_text_font(status_label, FONT_TINY, LV_PART_MAIN);
  lv_obj_set_pos(status_label, 18, 2);

  status_ip = lv_label_create(bar);
  lv_label_set_text(status_ip, "");
  lv_obj_set_style_text_color(status_ip, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_set_style_text_font(status_ip, FONT_TINY, LV_PART_MAIN);
  lv_obj_align(status_ip, LV_ALIGN_RIGHT_MID, -3, 0);
}

static lv_obj_t* make_panel(lv_obj_t* parent, lv_color_t bg) {
  lv_obj_t* p = lv_obj_create(parent);
  lv_obj_remove_style_all(p);
  lv_obj_set_size(p, PANEL_W, PANEL_H);
  lv_obj_set_pos(p, 0, STATUS_H);
  lv_obj_set_style_bg_color(p, bg, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(p, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(p, 6, LV_PART_MAIN);
  lv_obj_clear_flag(p, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
  return p;
}

// Tab 0: COSTO
static void build_cost_panel(lv_obj_t* p) {
  lv_obj_t* hdr = lv_label_create(p);
  lv_label_set_text(hdr, "OGGI");
  lv_obj_set_style_text_color(hdr, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(hdr, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 4);

  today_cost_lbl = lv_label_create(p);
  lv_label_set_text(today_cost_lbl, "$0.00");
  lv_obj_set_style_text_color(today_cost_lbl, lv_color_hex(0x00ff88), LV_PART_MAIN);
  lv_obj_set_style_text_font(today_cost_lbl, FONT_HUGE, LV_PART_MAIN);
  lv_obj_align(today_cost_lbl, LV_ALIGN_TOP_MID, 0, 26);

  lv_obj_t* sep = lv_obj_create(p);
  lv_obj_remove_style_all(sep);
  lv_obj_set_size(sep, 120, 1);
  lv_obj_set_style_bg_color(sep, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(sep, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* mh = lv_label_create(p);
  lv_label_set_text(mh, "MESE");
  lv_obj_set_style_text_color(mh, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(mh, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(mh, LV_ALIGN_CENTER, 0, 20);

  month_cost_lbl = lv_label_create(p);
  lv_label_set_text(month_cost_lbl, "$0.00");
  lv_obj_set_style_text_color(month_cost_lbl, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_set_style_text_font(month_cost_lbl, FONT_BIG, LV_PART_MAIN);
  lv_obj_align(month_cost_lbl, LV_ALIGN_CENTER, 0, 50);

  updated_lbl = lv_label_create(p);
  lv_label_set_text(updated_lbl, "");
  lv_obj_set_style_text_color(updated_lbl, lv_color_hex(0x666666), LV_PART_MAIN);
  lv_obj_set_style_text_font(updated_lbl, FONT_TINY, LV_PART_MAIN);
  lv_obj_align(updated_lbl, LV_ALIGN_BOTTOM_MID, 0, -4);
}

// Tab 1: FINESTRA 5h (Claude Code rate-limit window)
static void build_window_panel(lv_obj_t* p) {
  lv_obj_t* hdr = lv_label_create(p);
  lv_label_set_text(hdr, "FINESTRA 5h");
  lv_obj_set_style_text_color(hdr, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(hdr, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 4);

  // Numero messaggi grande, centrato
  win_msg_lbl = lv_label_create(p);
  lv_label_set_text(win_msg_lbl, "0");
  lv_obj_set_style_text_color(win_msg_lbl, lv_color_hex(0x00ccff), LV_PART_MAIN);
  lv_obj_set_style_text_font(win_msg_lbl, FONT_HUGE, LV_PART_MAIN);
  lv_obj_align(win_msg_lbl, LV_ALIGN_TOP_MID, 0, 24);

  lv_obj_t* mh = lv_label_create(p);
  lv_label_set_text(mh, "del limite 5h");
  lv_obj_set_style_text_color(mh, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_set_style_text_font(mh, FONT_TINY, LV_PART_MAIN);
  lv_obj_align(mh, LV_ALIGN_TOP_MID, 0, 62);

  // Costo finestra
  win_cost_lbl = lv_label_create(p);
  lv_label_set_text(win_cost_lbl, "$0.00");
  lv_obj_set_style_text_color(win_cost_lbl, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_set_style_text_font(win_cost_lbl, FONT_BIG, LV_PART_MAIN);
  lv_obj_align(win_cost_lbl, LV_ALIGN_TOP_MID, 0, 84);

  // Token in/out compatti
  win_tokens_lbl = lv_label_create(p);
  lv_label_set_text(win_tokens_lbl, "in 0  out 0");
  lv_obj_set_style_text_color(win_tokens_lbl, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(win_tokens_lbl, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(win_tokens_lbl, LV_ALIGN_TOP_MID, 0, 122);

  // Barra elapsed (tempo trascorso nella finestra)
  win_elapsed_bar = lv_bar_create(p);
  lv_obj_set_size(win_elapsed_bar, PANEL_W - 24, 16);
  lv_obj_set_pos(win_elapsed_bar, 12, 162);
  lv_bar_set_range(win_elapsed_bar, 0, 300);   // 300 minuti = 5h
  lv_bar_set_value(win_elapsed_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(win_elapsed_bar, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(win_elapsed_bar, lv_color_hex(0x00ff88), LV_PART_INDICATOR);

  // Countdown reset
  win_reset_lbl = lv_label_create(p);
  lv_label_set_text(win_reset_lbl, "nessuna finestra attiva");
  lv_obj_set_style_text_color(win_reset_lbl, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_set_style_text_font(win_reset_lbl, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(win_reset_lbl, LV_ALIGN_TOP_MID, 0, 190);
}

// Tab 2: GRAFICO 7 GIORNI
static void build_chart_panel(lv_obj_t* p) {
  lv_obj_t* hdr = lv_label_create(p);
  lv_label_set_text(hdr, "ULTIMI 7 GIORNI");
  lv_obj_set_style_text_color(hdr, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(hdr, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 4);

  chart_max_lbl = lv_label_create(p);
  lv_label_set_text(chart_max_lbl, "max $0");
  lv_obj_set_style_text_color(chart_max_lbl, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_set_style_text_font(chart_max_lbl, FONT_TINY, LV_PART_MAIN);
  lv_obj_align(chart_max_lbl, LV_ALIGN_TOP_RIGHT, -4, 22);

  chart = lv_chart_create(p);
  lv_obj_set_size(chart, PANEL_W - 16, PANEL_H - 70);
  lv_obj_set_pos(chart, 8, 40);
  lv_chart_set_type(chart, LV_CHART_TYPE_BAR);
  lv_chart_set_point_count(chart, 7);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
  lv_chart_set_div_line_count(chart, 4, 0);
  lv_obj_set_style_bg_color(chart, lv_color_hex(0x111111), LV_PART_MAIN);
  lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
  chart_series = lv_chart_add_series(chart, lv_color_hex(0x00ff88), LV_CHART_AXIS_PRIMARY_Y);

  // Labels giorno sotto le barre
  int chart_left = 8;
  int chart_w    = PANEL_W - 16;
  int chart_bottom = 40 + (PANEL_H - 70);
  int slot_w   = chart_w / 7;
  for (int i = 0; i < 7; i++) {
    chart_day_lbls[i] = lv_label_create(p);
    lv_label_set_text(chart_day_lbls[i], "--");
    lv_obj_set_style_text_color(chart_day_lbls[i], lv_color_hex(0xbbbbbb), LV_PART_MAIN);
    lv_obj_set_style_text_font(chart_day_lbls[i], FONT_TINY, LV_PART_MAIN);
    lv_obj_set_pos(chart_day_lbls[i], chart_left + slot_w * i + slot_w/2 - 8, chart_bottom + 2);
  }
}

// Tab 3: MODELLI
static void build_models_panel(lv_obj_t* p) {
  lv_obj_t* hdr = lv_label_create(p);
  lv_label_set_text(hdr, "MODELLI (MESE)");
  lv_obj_set_style_text_color(hdr, lv_color_hex(0xaaaaaa), LV_PART_MAIN);
  lv_obj_set_style_text_font(hdr, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 4);

  int row_h = 44;
  int top_y = 26;
  for (int i = 0; i < MODEL_ROWS; i++) {
    model_row[i] = lv_obj_create(p);
    lv_obj_remove_style_all(model_row[i]);
    lv_obj_set_size(model_row[i], PANEL_W - 12, row_h);
    lv_obj_set_pos(model_row[i], 6, top_y + i * (row_h + 4));
    lv_obj_set_style_bg_color(model_row[i], lv_color_hex(0x1a1a1a), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(model_row[i], LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(model_row[i], 4, LV_PART_MAIN);
    lv_obj_clear_flag(model_row[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(model_row[i], LV_OBJ_FLAG_HIDDEN);

    model_name_lbl[i] = lv_label_create(model_row[i]);
    lv_label_set_text(model_name_lbl[i], "");
    lv_obj_set_style_text_color(model_name_lbl[i], lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(model_name_lbl[i], FONT_SMALL, LV_PART_MAIN);
    lv_obj_set_pos(model_name_lbl[i], 6, 4);

    model_cost_lbl[i] = lv_label_create(model_row[i]);
    lv_label_set_text(model_cost_lbl[i], "");
    lv_obj_set_style_text_color(model_cost_lbl[i], lv_color_hex(0x00ff88), LV_PART_MAIN);
    lv_obj_set_style_text_font(model_cost_lbl[i], FONT_SMALL, LV_PART_MAIN);
    lv_obj_align(model_cost_lbl[i], LV_ALIGN_TOP_RIGHT, -6, 4);

    model_bar[i] = lv_bar_create(model_row[i]);
    lv_obj_set_size(model_bar[i], PANEL_W - 32, 6);
    lv_obj_set_pos(model_bar[i], 6, 28);
    lv_bar_set_range(model_bar[i], 0, 100);
    lv_bar_set_value(model_bar[i], 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(model_bar[i], lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(model_bar[i], lv_color_hex(0x00ccff), LV_PART_INDICATOR);
  }

  model_empty_lbl = lv_label_create(p);
  lv_label_set_text(model_empty_lbl, "Nessun dato");
  lv_obj_set_style_text_color(model_empty_lbl, lv_color_hex(0x666666), LV_PART_MAIN);
  lv_obj_set_style_text_font(model_empty_lbl, FONT_SMALL, LV_PART_MAIN);
  lv_obj_align(model_empty_lbl, LV_ALIGN_CENTER, 0, 0);
}

// Mostra solo il panel idx, nasconde gli altri
static void show_panel(uint8_t idx) {
  active_panel = idx % 4;
  for (uint8_t i = 0; i < 4; i++) {
    if (i == active_panel) lv_obj_clear_flag(panels[i], LV_OBJ_FLAG_HIDDEN);
    else                   lv_obj_add_flag(panels[i],   LV_OBJ_FLAG_HIDDEN);
  }
}

static void rotate_timer_cb(lv_timer_t*) {
  show_panel(active_panel + 1);
}

void UsageUI_Init() {
  lv_obj_t* scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

  make_status_bar(scr);

  panels[0] = make_panel(scr, lv_color_hex(0x000000));
  panels[1] = make_panel(scr, lv_color_hex(0x000000));
  panels[2] = make_panel(scr, lv_color_hex(0x000000));
  panels[3] = make_panel(scr, lv_color_hex(0x000000));

  build_cost_panel(panels[0]);
  build_window_panel(panels[1]);
  build_chart_panel(panels[2]);
  build_models_panel(panels[3]);

  show_panel(0);
  lv_timer_create(rotate_timer_cb, ROTATE_INTERVAL_MS, nullptr);
}

void UsageUI_SetIp(const char* ip) {
  if (status_ip && ip) lv_label_set_text(status_ip, ip);
}

void UsageUI_Update(const UsageData& d) {
  // --- Status bar ---
  if (d.online) {
    lv_obj_set_style_bg_color(status_dot, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_label_set_text(status_label, "ONLINE");
  } else {
    lv_obj_set_style_bg_color(status_dot, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_label_set_text(status_label, "OFFLINE");
  }

  // --- Costo ---
  char buf[40];
  fmt_money(buf, sizeof(buf), d.today_cost_usd);
  lv_label_set_text(today_cost_lbl, buf);
  fmt_money(buf, sizeof(buf), d.month_cost_usd);
  lv_label_set_text(month_cost_lbl, buf);

  if (d.last_update_ms > 0) {
    uint32_t age_s = (millis() - d.last_update_ms) / 1000;
    if (age_s < 999) lv_label_set_text_fmt(updated_lbl, "agg. %us fa  (#%u)", (unsigned)age_s, (unsigned)d.fetch_count);
    else             lv_label_set_text(updated_lbl, "in attesa...");
  } else {
    lv_label_set_text(updated_lbl, "in attesa di dati...");
  }

  // --- Finestra 5h ---
  if (d.win_active) {
    // % limite in grande (al posto del conteggio messaggi)
    uint16_t lp = d.win_limit_pct;
    if (lp > 999) lp = 999;
    lv_label_set_text_fmt(win_msg_lbl, "%u%%", (unsigned)lp);

    // colore numero grande: verde / arancio / rosso
    lv_color_t lpcol = lv_color_hex(0x00ff88);
    if (lp >= 90)      lpcol = lv_palette_main(LV_PALETTE_RED);
    else if (lp >= 70) lpcol = lv_palette_main(LV_PALETTE_ORANGE);
    lv_obj_set_style_text_color(win_msg_lbl, lpcol, LV_PART_MAIN);

    // costo + tetto piano
    if (d.win_limit_usd > 0.001f) {
      lv_label_set_text_fmt(win_cost_lbl, "$%.2f / $%.0f",
                            d.win_cost_usd, d.win_limit_usd);
    } else {
      fmt_money(buf, sizeof(buf), d.win_cost_usd);
      lv_label_set_text(win_cost_lbl, buf);
    }

    // messaggi assistant + token compatti
    char wi[16], wo[16];
    fmt_tokens(wi, sizeof(wi), d.win_tokens_in);
    fmt_tokens(wo, sizeof(wo), d.win_tokens_out);
    lv_label_set_text_fmt(win_tokens_lbl, "%u msg | out %s",
                          (unsigned)d.win_messages, wo);

    // barra = % limite (0-100, indicatore visivo dello stato)
    int pct = (lp > 100) ? 100 : (int)lp;
    lv_bar_set_value(win_elapsed_bar, pct, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(win_elapsed_bar, lpcol, LV_PART_INDICATOR);

    // Countdown reset
    uint16_t r = d.win_remaining_min;
    if (r == 0) {
      lv_label_set_text(win_reset_lbl, "reset in corso...");
    } else if (r < 60) {
      lv_label_set_text_fmt(win_reset_lbl, "reset tra %um", (unsigned)r);
    } else {
      lv_label_set_text_fmt(win_reset_lbl, "reset tra %uh %02um",
                            (unsigned)(r / 60), (unsigned)(r % 60));
    }
  } else {
    lv_label_set_text(win_msg_lbl, "0%");
    lv_obj_set_style_text_color(win_msg_lbl, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_label_set_text(win_cost_lbl, "$0.00");
    lv_label_set_text(win_tokens_lbl, "");
    lv_bar_set_value(win_elapsed_bar, 0, LV_ANIM_OFF);
    lv_label_set_text(win_reset_lbl, "nessuna finestra attiva");
  }

  // --- Grafico 7 giorni ---
  float maxv = 0.0f;
  for (int i = 0; i < 7; i++) if (d.last7_cost[i] > maxv) maxv = d.last7_cost[i];
  int scale_max = (int)(maxv * 1.1f) + 1;
  if (scale_max < 1) scale_max = 1;
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, scale_max);
  for (int i = 0; i < 7; i++) {
    int v = (int)(d.last7_cost[i] + 0.5f);
    lv_chart_set_value_by_id(chart, chart_series, i, v);
    lv_label_set_text(chart_day_lbls[i], d.last7_label[i]);
  }
  lv_chart_refresh(chart);
  if (maxv >= 1.0f) lv_label_set_text_fmt(chart_max_lbl, "max $%.0f", maxv);
  else              lv_label_set_text_fmt(chart_max_lbl, "max $%.2f", maxv);

  // --- Modelli ---
  uint8_t n = d.n_models;
  if (n > MODEL_ROWS) n = MODEL_ROWS;
  float total_cost = 0.0f;
  for (int i = 0; i < n; i++) total_cost += d.models[i].cost_usd;
  if (total_cost < 0.001f) total_cost = 1.0f;

  for (int i = 0; i < MODEL_ROWS; i++) {
    if (i < n) {
      lv_obj_clear_flag(model_row[i], LV_OBJ_FLAG_HIDDEN);
      char nm[24];
      short_model_name(d.models[i].name, nm, sizeof(nm));
      lv_label_set_text(model_name_lbl[i], nm);
      char mc[20];
      fmt_money(mc, sizeof(mc), d.models[i].cost_usd);
      lv_label_set_text(model_cost_lbl[i], mc);
      int p = (int)((d.models[i].cost_usd / total_cost) * 100.0f);
      lv_bar_set_value(model_bar[i], p, LV_ANIM_OFF);
    } else {
      lv_obj_add_flag(model_row[i], LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (n == 0) lv_obj_clear_flag(model_empty_lbl, LV_OBJ_FLAG_HIDDEN);
  else        lv_obj_add_flag(model_empty_lbl,   LV_OBJ_FLAG_HIDDEN);
}
