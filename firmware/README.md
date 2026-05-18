# Firmware — Claude Code Usage Monitor (PlatformIO)

Firmware per la board **Waveshare ESP32-S3-LCD-1.47** che mostra in tempo reale
il consumo Claude Code letto dal bridge Python `../bridge/`.

## Cosa visualizza

Display 172×320 con 4 viste che ruotano automaticamente ogni 6 secondi
(puoi mettere in pausa la rotazione col pulsante BOOT, vedi sotto):

1. **Costo** — `$ oggi` (grande, verde) + "ieri $X" + sparkline 7 giorni + `$ mese`.
2. **Finestra 5h** — `% del limite` (verde/ambra/rosso), `$cost / $limit`,
   due barre etichettate **Tempo** (viola, 0-300 min) e **Limite** (0-100 %),
   ETA-to-limit se rilevante, countdown reset.
3. **Ultimi 7 giorni** — bar chart costo giornaliero.
4. **Modelli (mese)** — top 5 modelli con costo e barra proporzionale.

In alto sempre una status bar con icona WiFi + ONLINE/OFFLINE + IP. Il LED RGB
onboard (GPIO 38) lampeggia viola ad ogni fetch riuscito, resta verde quando
online, rosso quando offline, blu in setup mode.

All'avvio uno **splash** (`Claude Code Usage v0.2.0`) resta visibile per ~2 s.

## Setup

Hai **due percorsi**: binary release (zero config a compile time) o source build
(metti tutto in `secrets.h`).

### Percorso A: binary release (captive portal)

1. `pio run -t upload` con `secrets.h` **vuota** (copia del template).
2. Al primo boot, la board apre un AP `ClaudeMonitor-XXYY` (XXYY = ultime 4
   hex del MAC) e mostra a video URL e nome rete.
3. Connetti il telefono all'AP → il popup captive si apre, oppure naviga
   manualmente a `http://192.168.4.1`.
4. Compila il form (WiFi, bridge IP/port/token, polling) e premi
   **Salva e collegati**. La board fa reboot ed entra in modalità normale.
5. Config persistita in NVS, non serve più ricompilare per cambiare WiFi/bridge.

### Percorso B: source build

```bash
cp src/secrets.h.template src/secrets.h
$EDITOR src/secrets.h   # riempi WIFI_SSID, WIFI_PASS, BRIDGE_HOST, BRIDGE_TOKEN
pio run -t upload
pio device monitor      # 115200 baud
```

I valori in `secrets.h` fanno da **fallback** quando NVS è vuota — quindi il
percorso source build resta retro-compatibile con v0.1.

## Pulsante BOOT (GPIO 0)

| Gesto | Durata | Effetto |
|---|---|---|
| Tap | <500 ms | Tab successivo; rotazione automatica in pausa 30 s |
| Long press | 500 ms – 5 s | Toggle rotazione automatica (persistito in NVS) |
| Very long | >5 s | Reset NVS + reboot in captive portal |

Polling-only (niente interrupt): GPIO 0 è anche strap pin di download mode, gli
interrupt qui sono pericolosi.

## Trigger captive portal

| Trigger | Effetto |
|---|---|
| NVS vuota **e** `secrets.h` vuota al boot | Portal mode automatico |
| Tieni BOOT >5 s | Wipe NVS + reboot in portal |
| WiFi STA fallisce 3× in 30 s | Drop in portal mode |

> Tenere BOOT *durante il reset* non funziona: il chip entra in ROM bootloader,
> non nel firmware. Usa il very-long-press a runtime.

## Build & flash

```bash
pio run                  # build only
pio run -t upload        # build + upload (auto-detect porta)
pio device monitor       # 115200 baud
```

Se l'upload fallisce: tieni premuto **BOOT** + premi **RESET** + rilascia **RESET**
+ rilascia **BOOT** per forzare il bootloader, poi riprova.

## Struttura PlatformIO

```
firmware/
├── platformio.ini             # board, PSRAM, lib_deps (lvgl, ArduinoJson)
├── .gitignore                 # ignora .pio/ e src/secrets.h
└── src/
    ├── main.cpp               # boot flow + button polling
    ├── secrets.h.template     # fallback config (lascia vuoto per portal)
    ├── secrets.h              # creato dall'utente, NON committare
    ├── lv_conf.h              # config LVGL 8.3 (Montserrat 22/28/32)
    ├── Display_ST7789.{cpp,h} # driver SPI + ST7789
    ├── LVGL_Driver.{cpp,h}    # init LVGL + flush callback
    ├── RGB_lamp.{cpp,h}       # driver NeoPixel GPIO 38
    ├── Wireless.{cpp,h}       # WiFi STA + auto-reconnect + fail counter
    ├── UsageClient.{cpp,h}    # GET /usage + Authorization header + ArduinoJson
    ├── UsageUI.{cpp,h}        # 4 panel, splash, portal, toast, fade tabs
    ├── Config.{cpp,h}         # config NVS (Preferences) + secrets.h fallback
    ├── Portal.{cpp,h}         # softAP + DNSServer + WebServer form
    └── Button.{cpp,h}         # BOOT button polled state machine
```

## Debug seriale

Output tipico al boot in modalità normale:

```
=== Claude Code Usage Monitor v0.2.0 ===
[Config] SSID="MioWifi" host="192.168.1.55" port=8787 poll=5000 rotate=1 token=(set)
[WiFi] begin SSID="MioWifi"
[WiFi] connected, IP=192.168.1.78 RSSI=-52 dBm
[UsageClient] polling http://192.168.1.55:8787/usage ogni 5000 ms (auth ON)
```

In captive portal:

```
[boot] config non valida — entro in portal mode
[Portal] AP=ClaudeMonitor-A1B2 IP=192.168.4.1 — apri http://192.168.4.1
```

Il token bridge non viene mai stampato (solo `(set)` / `(empty)`).

## Note tecniche

- **arduino-esp32 2.0.x compat**: `Backlight_Init` usa `ledcSetup`/`ledcAttachPin`
  con guard `#if ESP_ARDUINO_VERSION_MAJOR >= 3` per supportare anche la 3.x.
- **`PIN_NEOPIXEL` collision**: il core ESP32-S3 DevKitM-1 definisce
  `PIN_NEOPIXEL=48`; sulla Waveshare il LED è su 38. La macro è stata rinominata
  in `RGB_LED_PIN`.
- **NVS keys**: limite `Preferences` 15 char → usiamo `br_host`, `br_port`,
  `br_token`, `rotate` invece di nomi descrittivi.
- **Task layout**: polling HTTP su task FreeRTOS core 0; LVGL su main loop
  core 1; accesso ai dati condivisi via `xSemaphoreCreateMutex()`.
- **DNS captive trap**: il `DNSServer` su `*` instrada ogni hostname a
  `192.168.4.1`, il `WebServer.onNotFound` fa 302 a `/`. Funziona con i probe
  di iOS (captive.apple.com), Android (generate_204), Windows (connecttest.txt).
- **Memoria** (v0.2): ~1.22 MB flash, ~109 KB RAM. Ancora ~80 % di flash libero.

## Pin hardware riepilogo

Display ST7789 SPI: MOSI=45 SCLK=40 CS=42 DC=41 RST=39 BL=48 (PWM)
RGB LED: GPIO 38
BOOT button: GPIO 0 (con pull-up interna)
