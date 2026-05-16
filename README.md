# Claude Code Usage Monitor — ESP32-S3 Ambient Display

> A small desktop ambient display that shows your **Claude Code** usage in real time:
> spend, tokens, 7-day history, per-model breakdown, and — most importantly — the
> current **5-hour rate-limit window** with countdown to reset, mirroring what
> `claude.ai → Settings → Usage` shows you.

![platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![framework](https://img.shields.io/badge/framework-Arduino%20%7C%20PlatformIO-orange)
![ui](https://img.shields.io/badge/UI-LVGL%208.3-purple)
![license](https://img.shields.io/badge/license-GPLv3-green)

---

## Why

Claude Code (Pro / Max 5x / Max 20x) limits aren't a flat monthly bill — they're
a rolling **5-hour window** that resets 5h after your first message in that
window, plus a weekly cap. Knowing how much of that window you've burned is
useful for pacing your work, but checking the Anthropic dashboard requires
opening a browser. This project puts that number on a small dedicated screen
on your desk.

The transcript files Claude Code writes locally to `~/.claude/projects/` contain
all the data we need (`message.usage`, model, timestamps). A tiny Python bridge
parses them and exposes a JSON endpoint; the ESP32-S3 firmware polls it over
WiFi and renders four LVGL views on a 172×320 TFT.

## Features

- **5-hour window tracker** with countdown — closely matches the official
  Anthropic dashboard (within a few percent / few minutes).
- **Cost views**: today, this month, 7-day bar chart, top-N model breakdown.
- **Plan-aware**: pick `pro` / `max5` / `max20` or set a custom limit.
- **Pricing configurable** via `pricing.json` (USD per million tokens).
- **No cloud dependency**: bridge reads local transcript files; nothing leaves
  your LAN.
- **Robust**: dedup by `message.id` (Claude Code rewrites the same assistant
  message multiple times during tool calls — naive parsers count it 2-5×);
  anchors window start on the first `user` message (matching Anthropic's
  request-time accounting, not completion-time).
- **Auto-reconnect WiFi**, **OFFLINE indicator**, **last value persists** on
  bridge outage.
- ~33 % RAM, ~18 % flash on ESP32-S3 (8 MB PSRAM, 16 MB flash).

## Hardware

This firmware targets the [**Waveshare ESP32-S3-LCD-1.47**](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.47)
dev board:

| | |
|---|---|
| SoC      | ESP32-S3R8 (Xtensa LX7 dual-core 240 MHz) |
| Memory   | 512 KB SRAM, 8 MB OPI PSRAM, 16 MB flash |
| Display  | 1.47" TFT ST7789, 172×320, 262K colors, SPI |
| Wireless | WiFi 2.4 GHz, BLE 5 |
| RGB LED  | WS2812-compatible on GPIO 38 (status indicator) |
| USB      | Native USB CDC/JTAG (no extra serial chip) |

Should adapt to other ESP32-S3 boards with an ST7789 by tweaking pins in
`firmware/src/Display_ST7789.h`.

## Display in action

Real photos from the running firmware (the board is normally read with the
USB connector to one side — 320×172 effective landscape):

| Cost view | 7-day chart | Model breakdown |
|:---:|:---:|:---:|
| ![today and month cost](docs/screenshots/display-cost.jpg) | ![last 7 days bar chart](docs/screenshots/display-7days.jpg) | ![per-model cost](docs/screenshots/display-models.jpg) |
| `OGGI $58.01` / `MESE $193.40` | `ULTIMI 7 GIORNI` with auto-scaled bars | `Opus 4.7` vs `Haiku 4.5` with proportional bars |

Status bar at the top is always visible (`● ONLINE` + bridge endpoint), and
the onboard RGB LED mirrors the connection state.

The four views auto-rotate every 6 seconds:

1. **Costo** — `OGGI $X` (large, green) and `MESE $Y`, with `updated Xs ago` footer.
2. **Finestra 5h** *(the important one)* — current usage of the 5-hour rolling
   rate-limit window as a single big percentage (green / orange / red at
   70 % / 90 % thresholds), `$cost / $plan_limit`, `N msg | out tokens`, a
   progress bar, and `reset tra Xh Ym` countdown.
3. **Ultimi 7 giorni** — bar chart of daily cost over the past week,
   auto-scaled, today is the rightmost bar.
4. **Modelli (mese)** — top 5 models for the month with cost and a
   proportional cyan bar.

The status bar at the top of every view shows `●` (green ONLINE / red
OFFLINE), the bridge state, and your ESP32's LAN IP. The onboard RGB LED
on GPIO 38 mirrors that state and flashes purple on each successful poll.

## Quick start

> **Platform support**: developed and tested on **Linux**. The bridge uses
> only the Python 3.10+ standard library and PlatformIO is cross-platform,
> so the project is designed to work on **Windows and macOS** as well — the
> only differences are firewall config, the Python launcher (`py` vs
> `python3`), and serial port naming (`COMx` vs `/dev/ttyACM0`). See the
> per-step OS notes below.

### 1. Run the bridge on your computer

```bash
git clone https://github.com/rootedlab-code/claude-code-usage-monitor
cd claude-code-usage-monitor/bridge
```

| OS | Command |
|---|---|
| Linux / macOS | `python3 bridge.py --plan max5` |
| Windows (PowerShell or cmd) | `py bridge.py --plan max5` |

Output (look for the IP — you'll need it):

```
Claude Code Usage Bridge avviato
  ascolta su:   http://0.0.0.0:8787
  IP locale:    http://192.168.1.42:8787/usage
  budget mese:  500.00 USD
  limite 5h:    200.00 USD (max5)
```

Sanity check it works:

```bash
curl -s http://localhost:8787/usage | python3 -m json.tool
# Windows PowerShell:
# (Invoke-WebRequest http://localhost:8787/usage).Content | ConvertFrom-Json
```

> The bridge reads `~/.claude/projects/**/*.jsonl` via `pathlib.Path.home()`,
> which resolves to `/home/<user>/...` on Linux/macOS and
> `C:\Users\<user>\...` on Windows. No code change needed.

### 2. Open the firewall

The ESP32 needs to reach your computer on port 8787.

**Linux:**
```bash
sudo iptables -I INPUT -p tcp --dport 8787 -j ACCEPT
# or for ufw:     sudo ufw allow 8787/tcp
# or firewalld:   sudo firewall-cmd --add-port=8787/tcp
```

**macOS:** the built-in Application Firewall does not block inbound TCP for
local Python processes by default. If it prompts you, click "Allow".

**Windows** (run as Administrator):
```powershell
netsh advfirewall firewall add rule name="Claude Bridge" dir=in action=allow protocol=TCP localport=8787
```

### 3. Build and flash the firmware

Install [PlatformIO Core](https://platformio.org/install/cli) (recommended)
or the [PlatformIO VS Code extension](https://platformio.org/install/ide?install=vscode).
Both work on Windows, Linux, and macOS.

```bash
cd ../firmware
```

Create your `secrets.h`:

| OS | Command |
|---|---|
| Linux / macOS | `cp src/secrets.h.template src/secrets.h` |
| Windows (cmd) | `copy src\secrets.h.template src\secrets.h` |
| Windows (PowerShell) | `Copy-Item src\secrets.h.template src\secrets.h` |

Edit `src/secrets.h` and fill `WIFI_SSID`, `WIFI_PASS`, `BRIDGE_HOST`. Then:

```bash
pio run -t upload
pio device monitor        # 115200 baud — should log "WiFi connected"
```

> On Windows the ESP32-S3 appears as `COM3`, `COM4`, etc. — `pio` auto-detects
> it. The native USB-CDC driver is included in Windows 10/11; no extra driver
> needed.

If the upload fails: hold **BOOT**, tap **RESET**, release **RESET**, release
**BOOT** to enter download mode, then retry.

### 4. Watch the display

Within ~5 seconds of boot you should see numbers populate. The status dot
goes green and the RGB LED blinks purple on each successful poll.

## Configuration

### Bridge CLI flags

| Flag             | Default     | Description                                          |
|------------------|-------------|------------------------------------------------------|
| `--host`         | `0.0.0.0`   | Bind address                                         |
| `--port`         | `8787`      | TCP port                                             |
| `--plan`         | `max5`      | `pro` / `max5` / `max20` — sets 5h window limit      |
| `--plan-limit`   | (from plan) | Override 5h limit in USD                             |
| `--budget`       | `500`       | Monthly budget in USD (informative, used in mese)    |
| `--ttl`          | `2.0`       | In-memory cache TTL in seconds                       |

The defaults match Claude **Max 5x**. Override with `--plan max20` or
`--plan-limit 250` for a custom subscription.

### Plan presets (estimated 5h cost-equivalent limits)

| Plan           | Monthly | 5h limit (USD equiv) |
|----------------|---------|----------------------|
| Pro            | $20     | ~$40                 |
| Max 5x         | $100    | ~$200                |
| Max 20x        | $200    | ~$1000               |

These are *estimates* of what Anthropic's internal limit translates to in
API-equivalent dollars. They give a good visual indicator but won't be
exact to the percent; adjust `--plan-limit` if your dashboard shows a
consistent offset.

### Custom pricing

Edit `bridge/pricing.json` (USD per million tokens). Models are matched by
prefix, so `claude-opus-4-7-20251201` falls back to `claude-opus-4-7`. The
`_default` entry is used for unknown models.

### Auto-start the bridge

**Linux (systemd user service):**

```ini
# ~/.config/systemd/user/claude-bridge.service
[Unit]
Description=Claude Code Usage Bridge

[Service]
ExecStart=/usr/bin/python3 %h/path/to/claude-code-usage-monitor/bridge/bridge.py --plan max5
Restart=on-failure

[Install]
WantedBy=default.target
```

```bash
systemctl --user enable --now claude-bridge.service
```

**macOS (launchd):** create `~/Library/LaunchAgents/com.user.claude-bridge.plist`
with a `ProgramArguments` array pointing at `python3` + the script path, then
`launchctl load ~/Library/LaunchAgents/com.user.claude-bridge.plist`.

**Windows (Task Scheduler):**
```powershell
$action  = New-ScheduledTaskAction -Execute "py" -Argument "C:\path\to\claude-code-usage-monitor\bridge\bridge.py --plan max5"
$trigger = New-ScheduledTaskTrigger -AtLogOn
Register-ScheduledTask -TaskName "ClaudeBridge" -Action $action -Trigger $trigger -RunLevel Limited
```
Or place a `claude-bridge.bat` shortcut in `shell:startup` (`Win+R` → `shell:startup`).

## How it works

### Bridge

1. Walks `~/.claude/projects/**/*.jsonl` on every request (cached 2 s in RAM).
2. For each `type:"assistant"` row with `message.usage`:
   - **Dedup by `message.id`** — Claude Code rewrites the same message 2-5
     times in a single JSONL during tool-call iterations. Counting each
     occurrence would inflate totals by ~3×.
   - Compute cost via prefix-matched pricing × `(input + output + cache_*)`.
3. For the 5h window:
   - Collect timestamps of **both** `user` and `assistant` events from the
     last 10h (anchor points).
   - Walk them chronologically; whenever the current timestamp is more than
     5h after the active window start, open a new window.
   - The last `window_start` is the active session. `reset_at = start + 5h`.
   - Sum cost / tokens / messages of assistant events ≥ `window_start`.
4. Serve at `GET /usage` (JSON).

### Firmware

- **`Wireless`** — WiFi STA mode with `WiFiEvent` auto-reconnect.
- **`UsageClient`** — FreeRTOS task pinned to core 0, polls the bridge every
  5 s via `HTTPClient`, parses with `ArduinoJson` v7, stores in a shared
  struct guarded by `xSemaphoreCreateMutex()`.
- **`UsageUI`** — LVGL 8.3 on core 1. Four `lv_obj_t` panels (one visible
  at a time, swapped via `lv_obj_add_flag(LV_OBJ_FLAG_HIDDEN)` by a 6 s timer).
  UI refresh from the snapshot runs every 400 ms in `loop()`.
- **`Display_ST7789`** — original Waveshare driver, patched for both
  arduino-esp32 2.0.x and 3.x LEDC APIs (backlight PWM).

## Project structure

```
claude-code-usage-monitor/
├── LICENSE                        GPL v3
├── README.md                      this file
├── .gitignore
│
├── bridge/                        Python — runs on your PC
│   ├── bridge.py                  HTTP server, JSONL parser, aggregator
│   ├── pricing.json               editable model price table
│   └── README.md
│
├── firmware/                      ESP32-S3 — PlatformIO project
│   ├── platformio.ini
│   ├── README.md
│   └── src/
│       ├── main.cpp               setup() + loop()
│       ├── secrets.h.template     copy to secrets.h and edit
│       ├── lv_conf.h              LVGL config (Montserrat 22/28/32 enabled)
│       ├── Display_ST7789.{cpp,h} ST7789 SPI driver
│       ├── LVGL_Driver.{cpp,h}    LVGL init / flush callback
│       ├── RGB_lamp.{cpp,h}       NeoPixel driver (status LED)
│       ├── Wireless.{cpp,h}       WiFi STA + reconnect
│       ├── UsageClient.{cpp,h}    HTTP polling task + JSON parsing
│       └── UsageUI.{cpp,h}        LVGL UI (4 panels + status bar)
│
├── docs/
│   └── hardware/                  pin maps, reference notes
│
├── ESP32-S3-LCD-1.47-Demo/        upstream Waveshare demo (reference)
└── 1.47inch_LCD_Datasheet.pdf     ST7789 datasheet
```

## Troubleshooting

| Symptom                                | Likely cause / fix                                              |
|----------------------------------------|-----------------------------------------------------------------|
| `[UsageClient] HTTP -1` repeating      | Firewall on host. Open port 8787 (see Quick Start §2).          |
| Status bar stuck on OFFLINE            | Wrong `BRIDGE_HOST` IP; bridge not running; AP isolation on WiFi |
| WiFi never connects                    | Re-check SSID/password in `secrets.h`. 5 GHz-only networks unsupported. |
| Build error `ledcAttach not declared`  | arduino-esp32 2.0.x — guard already in code, ensure you're on PIO `espressif32 ≥ 6.x` |
| `PIN_NEOPIXEL redefined`               | Old core. Confirm `RGB_LED_PIN` is used (not `PIN_NEOPIXEL`).   |
| Bridge shows costs ~3× too high        | You're on an older bridge.py without dedup. Pull latest.        |
| 5h window % diverges from dashboard    | Tune `--plan-limit` to match your subscription's real cap.      |
| Upload fails / can't find /dev/ttyACM0 | Hold BOOT, tap RESET, release RESET, release BOOT.              |

## Building from scratch (Arduino IDE alternative)

PlatformIO is recommended but the project also builds in Arduino IDE 2.x:

1. Install **esp32 by Espressif Systems** ≥ 3.0.2 from Boards Manager.
2. Copy `ESP32-S3-LCD-1.47-Demo/Arduino/libraries/{lvgl,PNGdec}` to
   `~/Arduino/libraries/`.
3. Install **ArduinoJson** ≥ 7.0.0 from Library Manager.
4. Rename `firmware/src/main.cpp` → `firmware/firmware.ino` (or create a
   folder `firmware/firmware/firmware.ino` matching IDE conventions).
5. Board: **ESP32S3 Dev Module**. Settings:
   - USB CDC On Boot: Enabled
   - Flash Size: 16 MB
   - PSRAM: OPI PSRAM
   - Partition Scheme: 16M Flash (3MB APP / 9.9MB FATFS)

## Contributing

Issues and PRs welcome. A few ideas the project hasn't tackled yet:

- mDNS discovery so `secrets.h` doesn't need a hardcoded IP.
- Weekly limit indicator (Anthropic dashboard exposes both 5h *and* weekly).
- A captive-portal WiFi provisioning so first-time users skip `secrets.h`
  altogether.
- Port to other ESP32-S3 boards (T-Display-S3, M5StickC, etc).
- Token-based authentication on the bridge for hostile networks.
- Cost calculation refinement (1h-ephemeral cache pricing, server tool use,
  thinking tokens, ...).

When contributing, please run the bridge once with your own transcripts and
include the corrected output in the PR description if the change affects
parsing/cost.

## Acknowledgements

- **[Waveshare](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.47)** — board,
  reference schematics, original demos.
- **[LVGL](https://lvgl.io/)** — light-weight graphics library powering the UI.
- **[ArduinoJson](https://arduinojson.org/)** — JSON parsing on the MCU.
- **[Anthropic](https://www.anthropic.com/)** — Claude Code and the API model
  that makes this useful.

## License

This project is licensed under the **GNU General Public License v3.0**.
See [LICENSE](LICENSE) for the full text. In short: you're free to use,
modify, and redistribute it, but derivative works must remain GPL-licensed
and provide source.

The upstream Waveshare demo files under `ESP32-S3-LCD-1.47-Demo/` retain
their original licenses (typically MIT/BSD, see individual headers).
