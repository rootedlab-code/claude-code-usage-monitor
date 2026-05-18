# Contributing

Grazie per voler contribuire! Questo è un progetto hobby/community con un piccolo
codebase: tutto deve restare leggibile e pragmatico.

## Setup

### Bridge (Python)

Solo stdlib, niente venv o requirements. Python 3.10+:

```bash
python3 bridge/bridge.py --no-auth --port 8788   # locale per test rapidi
```

### Firmware (PlatformIO)

```bash
pip install --user platformio
cd firmware
cp src/secrets.h.template src/secrets.h
# riempi WIFI_SSID/PASS/BRIDGE_HOST/BRIDGE_PORT/BRIDGE_TOKEN
pio run                      # build
pio run -t upload            # upload via USB-C
pio device monitor -b 115200 # serial
```

## Workflow

1. Forka, crea un branch con prefisso d'area: `feat/portal-form`, `fix/null-deref`, `docs/quickstart`.
2. Un commit per intento. Messaggio commit nel formato `area: short imperative`,
   es. `bridge: gate /metrics behind same token`.
3. Apri una PR con descrizione `Summary` + `Test plan` (puoi copiare lo schema da
   esempi precedenti).

## Stile

- **Python**: PEP 8, niente formatter automatici imposti (preferisco diff piccoli a
  re-indent di massa). Type hints incoraggiati ma non obbligatori.
- **C++ / Arduino**: stile esistente — funzioni `Lower_snake_case` per le API
  pubbliche del modulo (es. `WiFi_Connect_STA`), `lowerSnake` per locali. Niente
  STL pesante: `String` di Arduino e tipi POSIX vanno bene.
- Niente librerie nuove se possiamo evitarle. Le builtin del core ESP32 e LVGL già
  caricato coprono quasi tutto.

## Cosa accettiamo volentieri

- Bug fix con steps to reproduce.
- Supporto a nuove board (mantenendo build esistente intatta).
- Provisioning più snello (es. BLE in alternativa al softAP).
- Localizzazioni (oggi UI in italiano, English appena qualcuno la propone).
- Documentazione: troubleshooting basato su esperienze reali, screenshot.

## Cosa preferiamo discutere prima

Apri prima una issue di proposta per:

- Aggiunte di dipendenze esterne.
- Cambi al protocollo `/usage` (rompono firmware esistente).
- Rewrites grossi (>500 LOC).

## Linee guida sicurezza

Non commettere mai:

- `firmware/src/secrets.h` (è già in `.gitignore`).
- File contenenti bearer token, chiavi API, password Wi-Fi reali.
- Transcript JSONL personali nei test.

Per segnalare vulnerabilità, segui [`SECURITY.md`](SECURITY.md) — non aprire
issue pubbliche.
