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

Output atteso (v0.2+):

```
Claude Code Usage Bridge avviato
  ascolta su:   http://0.0.0.0:8787
  IP locale:    http://192.168.x.x:8787/usage
  budget mese:  500.00 USD
  limite 5h:    200.00 USD (max5)
  ...
  auth:         bearer (token persistito in /home/you/.claude-code-usage/token)
  token:        aBcDeFgHiJ...xyz
  short:        aBcD...8XYz

Su ESP32, in secrets.h (o nel captive portal) imposta:
    #define BRIDGE_HOST   "192.168.x.x"
    #define BRIDGE_PORT   8787
    #define BRIDGE_TOKEN  "aBcDeFgHiJ...xyz"
```

## Autenticazione (v0.2+)

Dalla v0.2 il bridge richiede un **bearer token** su `/usage` e `/metrics`:

- Al primo avvio viene generato e salvato in `~/.claude-code-usage/token`
  (permessi `0600`, directory `0700`).
- Le run successive caricano lo stesso token.
- Puoi forzare un token esplicito con `--token <valore>` (es. per ambienti dove
  vuoi tu il controllo).
- Puoi disabilitare l'auth con `--no-auth` (sconsigliato: stampa un warning, e
  chiunque sulla LAN può leggere il consumo).
- `/health` resta sempre anonimo (utile come liveness probe).

## Verifica

```bash
# Token corrente
TOK=$(cat ~/.claude-code-usage/token)

# Test endpoint
curl -H "Authorization: Bearer $TOK" http://localhost:8787/usage | python3 -m json.tool
curl http://localhost:8787/health
curl -H "Authorization: Bearer $TOK" http://localhost:8787/metrics
```

## Endpoint `/metrics` (Prometheus)

Espone in formato testo Prometheus 0.0.4:

```
cc_cost_today_usd       <float>
cc_cost_month_usd       <float>
cc_window5h_pct         <int>
cc_window5h_cost_usd    <float>
cc_messages_total       <int>
```

Per scraping anonimo da Prometheus/Grafana: avvia il bridge con `--metrics-anon`
(in questo caso solo `/metrics` resta anonimo; `/usage` continua a richiedere token).

## Flag CLI

| Flag | Default | Descrizione |
|---|---|---|
| `--host` | `0.0.0.0` | Bind address |
| `--port` | `8787` | Porta TCP |
| `--budget` | `500.0` | Budget mensile USD (informativo) |
| `--plan` | `max5` | Preset limite 5h: `pro`/`max5`/`max20` |
| `--plan-limit` | (preset) | Override esplicito limite 5h USD |
| `--ttl` | `2.0` | Cache TTL secondi |
| `--token` | (auto) | Token esplicito (skip persistenza) |
| `--no-auth` | off | Disabilita auth (sconsigliato) |
| `--metrics-anon` | off | Espone `/metrics` senza token |

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
  "budget_monthly_usd": 500,
  "window5h": {
    "active": true,
    "messages": 49,
    "cost_usd": 10.62,
    "tokens_in": 123456,
    "tokens_out": 5678,
    "elapsed_min": 87,
    "remaining_min": 213,
    "limit_usd": 200.0,
    "limit_pct": 5,
    "start": "2026-05-18T13:15:00+02:00",
    "reset_at": "2026-05-18T18:15:00+02:00"
  }
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
