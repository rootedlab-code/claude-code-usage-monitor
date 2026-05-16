# ESP32-S3-LCD-1.47 - Documentazione Hardware di Riferimento

Fonte: https://www.waveshare.com/wiki/ESP32-S3-LCD-1.47

---

## 1. Panoramica

Board di sviluppo basata su ESP32-S3 con display LCD TFT da 1.47 pollici integrato,
slot per TF card e LED RGB. Connettivita WiFi e Bluetooth 5 con antenna ceramica onboard.

---

## 2. Specifiche Processore e Memoria

| Parametro        | Valore                                    |
|------------------|-------------------------------------------|
| SoC              | ESP32-S3R8                                |
| Architettura     | Xtensa 32-bit LX7 dual-core              |
| Frequenza max    | 240 MHz                                   |
| SRAM             | 512 KB                                    |
| ROM              | 384 KB                                    |
| PSRAM            | 8 MB                                      |
| Flash            | 16 MB                                     |
| USB              | Full-speed USB serial integrato           |

---

## 3. Display LCD

| Parametro        | Valore                                    |
|------------------|-------------------------------------------|
| Dimensione       | 1.47 pollici TFT                          |
| Risoluzione      | 172 x 320 pixel                           |
| Colori           | 262K (18-bit)                             |
| Controller       | ST7789                                    |
| Interfaccia      | SPI                                       |

---

## 4. Connettivita Wireless

| Parametro        | Valore                                    |
|------------------|-------------------------------------------|
| WiFi             | 2.4 GHz, 802.11 b/g/n                    |
| Bluetooth        | Bluetooth 5 (BLE)                         |
| Antenna          | Patch ceramica onboard                    |

---

## 5. Alimentazione

| Parametro        | Valore                                    |
|------------------|-------------------------------------------|
| Regolatore LDO   | ME6217C33M5G                              |
| Corrente max     | 800 mA                                    |
| Connettore       | USB Type-A                                |

---

## 6. Pinout GPIO - Mappatura Completa

### 6.1 Display LCD (SPI)

| Segnale          | GPIO | Descrizione                          |
|------------------|------|--------------------------------------|
| MOSI             | 45   | Master Out Slave In (dati SPI)       |
| SCLK             | 40   | Serial Clock (clock SPI)             |
| CS               | 42   | Chip Select (selezione chip)         |
| DC               | 41   | Data/Command (selezione dati/cmd)    |
| RST              | 39   | Reset display                        |
| BL               | 48   | Backlight (retroilluminazione)       |

### 6.2 TF Card (SD/MMC)

| Segnale          | GPIO | Descrizione                          |
|------------------|------|--------------------------------------|
| CMD              | 15   | Command line                         |
| SCK              | 14   | Clock                                |
| D0               | 16   | Data 0                               |
| D1               | 18   | Data 1                               |
| D2               | 17   | Data 2                               |
| D3               | 21   | Data 3                               |

### 6.3 Periferiche Onboard

| Componente       | GPIO | Descrizione                          |
|------------------|------|--------------------------------------|
| RGB LED          | 38   | LED RGB controllabile (WS2812-like)  |

---

## 7. Ambiente di Sviluppo

### 7.1 Framework Supportati

- **Arduino IDE** - richiede pacchetto `esp32 by Espressif Systems >= 3.0.2`
- **ESP-IDF** - tramite VSCode con plugin Espressif

### 7.2 Librerie Richieste (installazione offline)

| Libreria         | Versione | Utilizzo                             |
|------------------|----------|--------------------------------------|
| LVGL             | 8.3.10   | Libreria grafica per UI              |
| PNGdec           | 1.0.2    | Decodifica immagini PNG              |

### 7.3 Tool di Programmazione

- Flash Download Tool v3.9.5_0
- Supporto camera OV2640/OV5640 (opzionale)

---

## 8. Procedura di Download/Flash

### Modalita Download (se il flash fallisce):

1. Tenere premuto il pulsante **BOOT**
2. Premere contemporaneamente **RESET**
3. Rilasciare **RESET**
4. Rilasciare **BOOT**
5. Il dispositivo entra in modalita download

### Output USB:

- Abilitare `USB CDC On Boot` nelle impostazioni della board
- Oppure dichiarare `HWCDC` nel codice per output diretto via USB

---

## 9. Demo Disponibili (riferimento)

| Demo                        | Descrizione                                    |
|-----------------------------|------------------------------------------------|
| LVGL_Arduino                | Test funzionalita dispositivo con LVGL         |
| LCD_Image                   | Visualizzazione immagini PNG da TF card        |
| ESP32-S3-LCD-1.47-Test      | Test completo di tutte le periferiche          |

---

## 10. Risorse e Datasheet

- ESP32-S3 Technical Reference Manual (Espressif)
- Datasheet LCD 1.47" (ST7789)
- Schema elettrico ESP32-S3-LCD-1.47 (Waveshare)
- Demo code: ESP32-S3-LCD-1.47-Demo.zip

---

## 11. Note per Firmware Custom

### Configurazione SPI per LCD ST7789

```c
// Pin definitions per il nostro firmware
#define LCD_MOSI    45
#define LCD_SCLK    40
#define LCD_CS      42
#define LCD_DC      41
#define LCD_RST     39
#define LCD_BL      48

// Display parameters
#define LCD_WIDTH   172
#define LCD_HEIGHT  320
#define LCD_COLOR_DEPTH  16  // RGB565 tipicamente usato
```

### Configurazione SD Card

```c
// SD Card pin definitions (modalita SD/MMC 4-bit)
#define SD_CMD      15
#define SD_CLK      14
#define SD_D0       16
#define SD_D1       18
#define SD_D2       17
#define SD_D3       21
```

### RGB LED

```c
// NeoPixel / WS2812 RGB LED
#define RGB_LED_PIN 38
#define RGB_LED_NUM 1
```
