#pragma once
#include <lvgl.h>
#include "UsageClient.h"

// Costruisce l'interfaccia LVGL sulla schermata attiva. Va chiamata dopo Lvgl_Init().
void UsageUI_Init();

// Aggiorna i widget con uno snapshot. Va chiamata frequentemente (ad es. ogni 500 ms).
// Sicura da chiamare dal main loop dove gira lv_timer_handler().
void UsageUI_Update(const UsageData& d);

// Aggiorna il testo IP nella status bar.
void UsageUI_SetIp(const char* ip);
