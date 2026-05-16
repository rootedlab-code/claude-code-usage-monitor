# Firmware — Claude Code Usage Monitor (PlatformIO)

Firmware per la board **Waveshare ESP32-S3-LCD-1.47** che mostra in tempo reale
il consumo Claude Code letto dal bridge Python `../bridge/`.

## Cosa visualizza

Display 172×320 con 4 viste che ruotano automaticamente ogni 6 secondi:

1. **Costo** — `$ oggi` (grande) + `$ mese` + timestamp ultimo aggiornamento
2. **Token** — `IN / OUT / CACHE` del mese + barra budget mensile
3. **Grafico 7 giorni** — bar chart costo giornaliero
4. **Modelli** — top 5 modelli con costo e quota proporzionale

In alto sempre una status bar con LED ONLINE/OFFLINE e IP del bridge.
Il LED RGB onboard (GPIO 38) lampeggia viola ad ogni fetch riuscito, resta verde
quando online, rosso quando il bridge non risponde.

## Struttura PlatformIO

```
firmware/
├── platformio.ini             # configurazione board / lib_deps
├── .gitignore                 # ignora .pio/ e src/secrets.h
└── src/
    ├── main.cpp               # setup() + loop()
    ├── secrets.h.template     # template credenziali (copia in secrets.h)
    ├── secrets.h              # ← creato dall'utente, NON committare
    ├── lv_conf.h              # config LVGL 8.3.9 con font 22/28/32 abilitati
    ├── Display_ST7789.{cpp,h} # driver SPI + ST7789 (dal demo Waveshare)
    ├── LVGL_Driver.{cpp,h}    # init LVGL + flush callback
    ├── RGB_lamp.{cpp,h}       # driver NeoPixel GPIO 38
    ├── Wireless.{cpp,h}       # WiFi STA con auto-reconnect
    ├── UsageClient.{cpp,h}    # task FreeRTOS: GET /usage + ArduinoJson
    └── UsageUI.{cpp,h}        # 4 panel LVGL + status bar + auto-rotate
```

## Setup

### 1. Avviare il bridge sul PC (terminale dedicato)

```bash
cd ../bridge
python3 bridge.py
```

Il bridge stampa l'IP da inserire in `secrets.h`. Es:

```
IP locale:    http://192.168.1.55:8787/usage
```

### 2. Configurare `secrets.h`

```bash
cp src/secrets.h.template src/secrets.h
$EDITOR src/secrets.h
```

Riempi `WIFI_SSID`, `WIFI_PASS`, `BRIDGE_HOST` (IP del PC), `BRIDGE_PORT`.

### 3. Build & Upload

```bash
# build only
pio run

# build + upload (rileva la porta automaticamente)
pio run -t upload

# Serial monitor a 115200 baud
pio device monitor
```

Se l'upload fallisce: tieni premuto **BOOT** + premi **RESET** + rilascia **RESET**
+ rilascia **BOOT** per forzare il bootloader, poi riprova.

## Debug seriale

Serial Monitor a **115200 baud**. Output atteso al boot:

```
=== Claude Code Usage Monitor ===
[WiFi] begin SSID="MioWifi"
[WiFi] connected, IP=192.168.1.78 RSSI=-52 dBm
[UsageClient] polling http://192.168.1.55:8787/usage ogni 5000 ms
```

## Note tecniche

- **arduino-esp32 2.0.x compat**: `Backlight_Init` usa `ledcSetup`/`ledcAttachPin`
  con guard `#if ESP_ARDUINO_VERSION_MAJOR >= 3` per supportare anche la 3.x.
- **`PIN_NEOPIXEL` collision**: il core ESP32-S3 DevKitM-1 definisce
  `PIN_NEOPIXEL=48`; sulla Waveshare il LED è su 38. La macro è stata rinominata
  in `RGB_LED_PIN` per evitare ridefinizioni.
- **LVGL config**: `src/lv_conf.h` ha già abilitati Montserrat 22/28/32 e
  `LV_USE_LOG=1`. Il flag `-DLV_CONF_INCLUDE_SIMPLE` istruisce LVGL a leggere
  il `lv_conf.h` locale invece del template di default.
- **Task layout**: il polling HTTP gira su un task FreeRTOS dedicato pinnato a
  core 0, LVGL gira sul main loop (core 1) — accesso ai dati condivisi via mutex.
- **Memoria**: build attuale ~1.2 MB flash, ~108 KB RAM (margine ampio).

## Pin hardware riepilogo

Display ST7789 SPI: MOSI=45 SCLK=40 CS=42 DC=41 RST=39 BL=48 (PWM)
RGB LED: GPIO 38
