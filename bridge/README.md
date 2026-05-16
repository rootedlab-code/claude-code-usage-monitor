# Claude Code Usage Bridge

Server HTTP locale (Python 3.10+, zero dipendenze) che legge i transcript JSONL di
Claude Code in `~/.claude/projects/` e li espone come JSON per il firmware
ESP32-S3-LCD-1.47 (`../firmware`).

## Avvio

```bash
python3 bridge.py
# avvio con porta/budget custom:
python3 bridge.py --port 9000 --budget 200
```

Output atteso:

```
Claude Code Usage Bridge avviato
  ascolta su:   http://0.0.0.0:8787
  IP locale:    http://192.168.x.x:8787/usage
  budget mese:  500.00 USD
  ...
Su ESP32, in secrets.h imposta:
    #define BRIDGE_HOST  "192.168.x.x"
    #define BRIDGE_PORT  8787
```

## Verifica

```bash
curl http://localhost:8787/usage | python3 -m json.tool
curl http://localhost:8787/health
```

## Schema risposta `/usage`

```json
{
  "ts": "2026-05-16T15:23:10+02:00",
  "today":  {"cost_usd": 12.34, "tokens_in": 1234567, "tokens_out": 56789, "cache_read": 9876543},
  "month":  {"cost_usd": 234.56, "tokens_in": ..., "tokens_out": ..., "cache_read": ...},
  "last7":  [{"date": "2026-05-10", "cost_usd": 3.21}, ...],
  "by_model": [
    {"name": "claude-opus-4-7", "cost_usd": 150.20, "tokens_in": ..., "tokens_out": ...},
    ...
  ],
  "budget_monthly_usd": 500
}
```

`last7` ha sempre 7 entries (oggi è l'ultima); `by_model` è ordinato per costo decrescente.

## Personalizzare i prezzi

Modifica `pricing.json` (USD per 1 milione di token). I modelli sono matchati per
prefisso, quindi `claude-opus-4-7-20251201` ricade su `claude-opus-4-7`. La chiave
`_default` viene usata per modelli ignoti.

## Avvio automatico (Linux, systemd user)

```ini
# ~/.config/systemd/user/claude-bridge.service
[Unit]
Description=Claude Code Usage Bridge

[Service]
ExecStart=/usr/bin/python3 %h/Desktop/claude\x20code\x20usage/bridge/bridge.py
Restart=on-failure

[Install]
WantedBy=default.target
```

```bash
systemctl --user enable --now claude-bridge.service
```

## Note

- I JSONL sono letti read-only ad ogni richiesta (con cache TTL 2s in RAM per non
  rileggere migliaia di righe ad ogni poll dell'ESP32).
- Funziona anche se Claude Code non è in esecuzione: i transcript restano su disco.
- Non viene esposto nessun dato sensibile dei prompt — solo aggregati di costo/token.
