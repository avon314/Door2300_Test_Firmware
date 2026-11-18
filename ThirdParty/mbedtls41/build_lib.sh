#!/bin/bash
# Builds libmbedtls41.a (Mbed TLS 4.1.1 + TF-PSA-Crypto 1.1.1) for TFW1_BMS_SERVER_R1 (same as MM-01) with the Atmel Studio
# ARM toolchain and the mm01_* config headers. Run from Git Bash:
#     ./build_lib.sh Debug      -> Debug/libmbedtls41.a    (defines DEBUG: debug/error strings, pool stats)
#     ./build_lib.sh Release    -> Release/libmbedtls41.a
#     ./build_lib.sh            -> both
# Rebuild BOTH whenever src/config/mbedtls/*.h, tls_cfg_guard.* or the vendored sources change.
# The app links the library matching its configuration (see TFW1_BMS_SERVER_R1.cproj library search path);
# a Debug/Release mismatch fails at link time (undefined mm_tls_cfg_guard_*).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
M="$HERE/../mbedtls-4.1.1"; T="$M/tf-psa-crypto"; APP="$HERE/../../TFW1_BMS_SERVER_R1"
BIN="${ARM_GCC_BIN:-/c/Program Files (x86)/Atmel/Studio/7.0/toolchain/arm/arm-gnu-toolchain/bin}"

build() {
  CFG="$1"; OUT="$HERE/$CFG"; OBJ="$HERE/obj_$CFG"
  rm -rf "$OBJ"; mkdir -p "$OBJ" "$OUT"
  DEF=(-DNDEBUG); [ "$CFG" = Debug ] && DEF=(-DDEBUG)
  FL=(-x c -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Os -std=gnu99
      -ffunction-sections -fdata-sections -mlong-calls -g -Wall -c "${DEF[@]}"
      '-DMBEDTLS_CONFIG_FILE="mm01_mbedtls_config.h"' '-DTF_PSA_CRYPTO_CONFIG_FILE="mm01_crypto_config.h"'
      -I"$APP/src/config/mbedtls" -I"$APP/src/network"
      -I"$M/include" -I"$T/include" -I"$T/drivers/builtin/include"
      -I"$M/library" -I"$T/core" -I"$T/dispatch" -I"$T/extras" -I"$T/platform" -I"$T/utilities"
      -I"$T/drivers/builtin/src" -I"$T/drivers/p256-m" -I"$T/drivers/everest/include" -I"$T/drivers/pqcp/include")
  fail=0; n=0
  for f in "$M"/library/*.c "$T"/core/*.c "$T"/drivers/builtin/src/*.c "$T"/drivers/p256-m/*.c \
           "$T"/drivers/p256-m/p256-m/*.c "$T"/extras/*.c "$T"/platform/*.c "$T"/utilities/*.c \
           "$APP/src/network/tls_cfg_guard.c"; do
    case "$(basename "$f")" in net_sockets.c|timing.c) continue;; esac
    o="$OBJ/$(basename "$(dirname "$f")")_$(basename "$f" .c).o"
    if "$BIN/arm-none-eabi-gcc.exe" "${FL[@]}" -o "$o" "$f"; then n=$((n+1)); else echo "FAILED: $f"; fail=1; fi
  done
  [ $fail -eq 0 ] || { echo "$CFG: build failed"; return 1; }
  rm -f "$OUT/libmbedtls41.a"
  "$BIN/arm-none-eabi-ar.exe" rcs "$OUT/libmbedtls41.a" "$OBJ"/*.o && rm -rf "$OBJ"
  echo "$CFG: $n objects -> $OUT/libmbedtls41.a"
}

case "${1:-all}" in
  Debug|Release) build "$1" ;;
  all) build Debug && build Release ;;
  *) echo "usage: $0 [Debug|Release]"; exit 2 ;;
esac
