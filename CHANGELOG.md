# Changelog

Tutte le modifiche notevoli a questo progetto sono documentate qui. Formato basato su
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versioning [SemVer](https://semver.org/lang/it/).

## [Unreleased]

## [0.2.0] — 2026-05-18

### Added — Sicurezza
- Bridge: bearer token obbligatorio su `/usage` e `/metrics` (`hmac.compare_digest`,
  costante in tempo). Generato in `~/.claude-code-usage/token` al primo avvio
  con permessi `0600`. Flag `--token`, `--no-auth`, `--metrics-anon`.
- Endpoint `/health` resta anonimo per liveness probes.
- Endpoint `/metrics` in formato Prometheus 0.0.4 (cost today/month, window5h %, messages).
- `SECURITY.md` con threat model e contatto per disclosure.
- Firmware: invia `Authorization: Bearer ...` automaticamente quando `BRIDGE_TOKEN`
  è valorizzato; log esplicito su HTTP 401; il token non viene mai stampato su Serial.

### Added — UX
- Captive portal di provisioning runtime sull'ESP32: al primo boot (o tenendo
  premuto BOOT >5s) il device crea un AP `ClaudeMonitor-XXYY` con form web
  per WiFi, bridge host/port, token, cadenza poll. Config persistito in NVS.
- Boot splash con logo, titolo, versione.
- Palette colore unificata con accenti semantici (verde costo, ciano token,
  ambra warning, rosso danger, viola tempo).
- Icone LV_SYMBOL su ogni header di tab.
- Tab "Finestra 5h": ora con due barre etichettate — `Tempo` (0-300 min, viola)
  e `Limite` (0-100%, green→amber→red). Risolta l'ambiguità visiva di v0.1.
- Sparkline 7 giorni in basso a destra del Tab Costo.
- Label "ieri $X.YZ" sotto la cifra OGGI.
- Fade transitions tra tab.
- ETA-to-limit sul Tab Finestra 5h: estrapolazione del momento in cui finirai
  il budget al ritmo corrente (ambra >30 min, rosso <30 min).
- Pulsante BOOT (GPIO 0) per navigazione manuale:
  - **Tap** (<500 ms): tab successivo, rotazione automatica in pausa 30s.
  - **Long press** (1–5 s): toggle rotazione automatica (persistente).
  - **Very long** (>5 s): reset NVS + reboot in captive portal.

### Added — Repo
- `CONTRIBUTING.md` con linee guida.
- `CHANGELOG.md` (questo file).
- Template per issue (`.github/ISSUE_TEMPLATE/`).
- GitHub Actions workflow (`.github/workflows/build.yml`): py_compile per il
  bridge, `pio run` per il firmware su ogni push/PR.

### Changed
- `secrets.h.template`: aggiunto `BRIDGE_TOKEN`. I valori vuoti sono trattati
  come fallback per il captive portal — utenti source-build possono ancora
  riempire tutto a compile-time.
- README.md: nuova sezione "Setup (binary release)" come path principale,
  "Build from source" rimane per chi vuole compilare.

### Migration v0.1 → v0.2
- Utenti che hanno già `secrets.h` riempito: aggiungete `#define BRIDGE_TOKEN ""`
  (o il valore stampato dal bridge), poi `pio run -t upload`. Tutto resta funzionante.
- Utenti del bridge: la prima `curl` dopo l'upgrade darà `401`. Aggiungete
  `-H "Authorization: Bearer $(cat ~/.claude-code-usage/token)"`, o avviate con
  `--no-auth` come comportamento legacy (warning a video).

## [0.1.0] — 2026-05-16

### Added
- Bridge Python stdlib-only che legge `~/.claude/projects/**/*.jsonl` ed espone
  `/usage` con aggregati `today`, `month`, `last7`, `by_model`, `window5h`.
- Deduplica per `message.id` (risolve ~3× inflazione costi da resume/compact).
- Anchor finestra 5h sui timestamp user (allinea con dashboard Anthropic).
- Preset piano `pro` / `max5` / `max20`, override con `--plan-limit`.
- Pricing override via `pricing.json`.
- Firmware Waveshare ESP32-S3-LCD-1.47 con LVGL 8.3, 4 tab auto-rotanti,
  WiFi STA con auto-reconnect, polling HTTP 5s, status LED RGB.
