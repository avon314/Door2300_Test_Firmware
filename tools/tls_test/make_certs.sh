#!/bin/sh
# Creates a self-signed server certificate for testing MM-01 HTTPS.
# The device uses VERIFY_NONE, so the issuer does not matter. The key type must match
# the firmware's cipher suite:
#   rsa   (default) - RSA-2048, sha256WithRSAEncryption  -> ECDHE-RSA-AES128-GCM-SHA256 (current firmware)
#   ecdsa           - ECDSA P-256, ecdsa-with-SHA256      -> ECDHE-ECDSA-AES128-GCM-SHA256 (earlier profile)
# Usage: ./make_certs.sh [hostname] [rsa|ecdsa]      (default: mm01-test.local rsa)
# Output: srv.key, srv.pem (in the current directory). Requires OpenSSL 1.1.1+.
set -e
HOST="${1:-mm01-test.local}"
TYPE="${2:-rsa}"

if [ "$TYPE" = "ecdsa" ]; then
    openssl ecparam -name prime256v1 -genkey -noout -out srv.key
else
    openssl genrsa -out srv.key 2048
fi

openssl req -x509 -new -key srv.key -sha256 -days 3650 \
    -subj "/CN=${HOST}" \
    -addext "subjectAltName=DNS:${HOST}" \
    -addext "keyUsage=critical,digitalSignature" \
    -addext "extendedKeyUsage=serverAuth" \
    -out srv.pem

openssl x509 -in srv.pem -noout -subject -dates -fingerprint -sha256
echo "Key type: ${TYPE}. The SHA-256 fingerprint above is what the device prints (debug builds) after a full handshake."
