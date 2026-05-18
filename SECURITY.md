# Security policy

## Threat model

Claude Code Usage Monitor è progettato per essere usato in una **rete locale fidata**
(tipicamente la rete di casa o dell'ufficio). Il bridge gira sul tuo computer, l'ESP32
sulla stessa LAN polla `/usage` ogni pochi secondi.

Cosa difendiamo:

- **Lettura non autorizzata dei dati di consumo da altri client sulla LAN.** Dalla
  v0.2 ogni richiesta a `/usage` e `/metrics` deve presentare un bearer token
  generato al primo avvio e salvato in `~/.claude-code-usage/token` (permessi
  `0600`, directory `0700`).
- **Scan opportunistici** di porte 8787 sulla LAN: senza token rispondiamo `401`.

Cosa NON difendiamo (out of scope in v0.2):

- **Adversari con shell access sul PC dove gira il bridge.** Possono leggere il
  token dal filesystem o leggere direttamente i transcript JSONL.
- **Intercettazione passiva sulla LAN** (Wi-Fi senza WPA2/3, sniffing su switch
  non-managed): il traffico è in HTTP plain. TLS è previsto per v0.3.
- **Esposizione su Internet.** Non esporre il bridge a Internet senza un reverse
  proxy TLS-terminating davanti — il bearer token non sostituisce HTTPS.
  Workaround: metti nginx/Caddy davanti a `127.0.0.1:8787` con cert Let's Encrypt
  o tunnel SSH/Tailscale.
- **Prompt content leakage.** Il bridge espone solo dati aggregati (cost, token
  counts, model breakdown). I prompt e le response complete restano nei file
  JSONL e non vengono mai trasmessi.

## Versioni supportate

| Versione | Supportata |
|----------|------------|
| 0.2.x    | sì         |
| 0.1.x    | no — passa a 0.2.x |

## Segnalare una vulnerabilità

Email: **rootedlab@proton.me** con subject `[cc-monitor security]`.

- Acknowledgement entro 7 giorni.
- Fix target: 30 giorni per medium-high severity.
- Per favore non aprire una issue pubblica prima della disclosure coordinata.

Apprezziamo la responsible disclosure e ringraziamo chiunque ci aiuti a tenere
il progetto sicuro per la community.
