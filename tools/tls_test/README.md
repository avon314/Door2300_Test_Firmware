# HTTPS test tools (TFW1_BMS_SERVER_R1)

PC-side tools for testing the device's TLS 1.2 client. They come from the MM-01 project; the firmware here uses the same TLS code and the same profile B (`ECDHE-RSA-AES128-GCM-SHA256`). Run them on Linux, macOS, or Windows with Git Bash and Python 3.

## Steps

1. **Make a server certificate:** run `./make_certs.sh tfw1-test.local rsa`. This writes `srv.key` and `srv.pem`. Use `rsa` (the default): this firmware is built for RSA server certificates.
2. **Provision the device seed (once per device).** Builds with `MM_TLS_TEST_FIXED_SEED=1` (the default in `conf_https.h`) use a compiled-in seed and skip this step. With `MM_TLS_TEST_FIXED_SEED=0`, send this on the debug UART (9600 8N1 on this board, `CONF_UART_BAUDRATE`):
   ```
   SEED <64 hex chars>
   ```
   Generate the hex on the PC with `openssl rand -hex 32`. Use a new value for every device.
   - The device replies `SEED:OK`.
   - `SEED?` reports `SEED:PRESENT` or `SEED:MISSING`.
   - Until a seed exists, HTTPS stays disabled and the device logs `TLS: no entropy seed provisioned`.
3. **Run a server**, using either of these:
   - `./s_server.sh 8443`: handshake only, with the full message trace.
   - `python3 post_echo.py 8443`: the real POST and JSON round trip.
4. **Point the device at the server.** The endpoint is compile-time in this firmware. Add this project symbol in Atmel Studio (both configurations) and rebuild:
   ```
   HTTPS_SERVER_ENDPOINT=\"https://<PC-IP>:8443/\"
   ```
   Without it, the device posts to `https://httpbin.org/post` (a public echo server; it needs internet access through the gateway in `conf_eth.h`, and DNS at 8.8.8.8).

The device posts every `HTTPS_POLL_INTERVAL_MS` (10 s by default).

## What to check

| Check | Where |
|---|---|
| Cipher `ECDHE-RSA-AES128-GCM-SHA256` | s_server output, or the cipher post_echo prints |
| `max_fragment_length` (2048) in the ClientHello | `s_server -msg` trace |
| Full handshake time | device UART `TLS: handshake ms = ...` (first poll) |
| Resumed handshake on later polls | s_server shows `Reused session`; device handshake time much shorter |
| Server cert fingerprint (Debug builds) | device `TLS: server cert SHA-256 ...` matches `make_certs.sh` output |
| TLS pool peak, task stack, heaps (`MM_MEM_STATS=1`) | device `TLS pool used/peak/size` and the lines before it |

## Expected failures

The device should log a failure and send nothing in each of these cases:
- **ECDSA server certificate** (`make_certs.sh <host> ecdsa`): no common cipher suite.
- **TLS 1.3-only server:** `openssl s_server -tls1_3 ...`.
- **No seed provisioned** (with `MM_TLS_TEST_FIXED_SEED=0`): the device logs `TLS: no entropy seed provisioned`.

Because the device uses `VERIFY_NONE`, it **accepts** a self-signed, wrong-hostname or expired RSA certificate. That is the decided behaviour (guide section 10).
