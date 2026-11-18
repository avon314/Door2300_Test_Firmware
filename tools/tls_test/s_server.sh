#!/bin/sh
# Minimal TLS 1.2 server with exactly the MM-01 profile, for handshake tests.
# Shows every TLS message (-msg) so you can check:
#   - cipher ECDHE-RSA-AES128-GCM-SHA256, group P-256
#   - max_fragment_length extension (2048) in the ClientHello
#   - "Reused session" on the 2nd and later polls (session ticket resumption)
# Usage: ./s_server.sh [port] [rsa|ecdsa]   (default 8443 rsa; the certificate must match, see make_certs.sh)
# Point the device endpoint at https://<PC-IP>:<port>/
# -www answers every request with a status page; use post_echo.py for real POST/JSON tests.
PORT="${1:-8443}"
TYPE="${2:-rsa}"
if [ "$TYPE" = "ecdsa" ]; then CIPHER=ECDHE-ECDSA-AES128-GCM-SHA256; else CIPHER=ECDHE-RSA-AES128-GCM-SHA256; fi
openssl s_server -accept "$PORT" -cert srv.pem -key srv.key \
    -tls1_2 -cipher "$CIPHER" -curves prime256v1 \
    -msg -state -www
