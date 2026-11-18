#!/usr/bin/env python3
"""HTTPS POST test server for MM-01.

Accepts the device's POST to /api/method/avon_iot.apis.iot.endpoint, prints the JSON body,
and answers with a small JSON reply (edit REPLY to test WR/ST handling).
TLS profile matches the device: TLS 1.2, ECDHE-RSA-AES128-GCM-SHA256 (RSA certificate from
make_certs.sh), ECDHE on P-256. An ECDSA certificate still works for the earlier ECDSA firmware.

Usage:  python3 post_echo.py [port]          (default 8443; needs srv.pem/srv.key from make_certs.sh)
Device endpoint:  https://<PC-IP>:<port>/
"""
import http.server
import json
import os
import ssl
import sys

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8443
REPLY = {"message": {"status": "ok"}}


class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length)
        print("\n--- POST %s from %s (TLS %s, %s)" % (
            self.path, self.client_address[0], self.connection.version(), self.connection.cipher()[0]))
        print("Host:", self.headers.get("Host"))
        try:
            print(json.dumps(json.loads(body), indent=2))
        except ValueError:
            print(body)
        out = json.dumps(REPLY).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(out)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(out)


ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ctx.minimum_version = ssl.TLSVersion.TLSv1_2
ctx.maximum_version = ssl.TLSVersion.TLSv1_2
ctx.set_ciphers("ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256")  # matches the srv.pem key type
ctx.set_ecdh_curve("prime256v1")
ctx.load_cert_chain("srv.pem", "srv.key")
# Wireshark decryption: set SSLKEYLOGFILE=<path> before starting; the TLS session keys
# are appended to that file (NSS key log format). Bench use only - the file decrypts
# every captured session.
if os.environ.get("SSLKEYLOGFILE"):
    ctx.keylog_filename = os.environ["SSLKEYLOGFILE"]
    print("TLS key log:", ctx.keylog_filename)

srv = http.server.HTTPServer(("0.0.0.0", PORT), Handler)
srv.socket = ctx.wrap_socket(srv.socket, server_side=True)
print("Listening on https://0.0.0.0:%d/" % PORT)
srv.serve_forever()
