# libmbedtls41: prebuilt Mbed TLS for TFW1_BMS_SERVER_R1

`Debug/libmbedtls41.a` and `Release/libmbedtls41.a` are Mbed TLS 4.1.1 (`../mbedtls-4.1.1`) compiled at `-Os` for the Cortex-M4 (softfp, fpv4-sp-d16) with this configuration (two suites: ECDHE-ECDSA-AES128-GCM-SHA256 and ECDHE-RSA-AES128-GCM-SHA256, so servers with either an ECDSA or an RSA certificate work; both enabled 2026-11-18):
- `TFW1_BMS_SERVER_R1/src/config/mbedtls/mm01_mbedtls_config.h`
- `TFW1_BMS_SERVER_R1/src/config/mbedtls/mm01_crypto_config.h`

The libraries were first copied from the MM-01 project, then rebuilt here: on 2026-11-17 with the `ssl.h` patch below, and on 2026-11-18 with both cipher suites enabled. The config headers therefore no longer match MM-01's. They keep their `mm01_` names only so that the build scripts and include paths stay the same.

The app project links the library from the folder that matches its configuration (`../../ThirdParty/mbedtls41/$(Configuration)`).

The library is prebuilt because Atmel Studio 7 cannot load a separate library project for it (guide pitfall 6). Building the library is therefore not part of the IDE build.

## Local patch to the vendored Mbed TLS (2026-11-17)
`../mbedtls-4.1.1/include/mbedtls/ssl.h`, `union mbedtls_ssl_premaster_secret`: `_pms_ecdh` is sized with `PSA_RAW_KEY_AGREEMENT_OUTPUT_MAX_SIZE` instead of `MBEDTLS_ECP_MAX_BYTES`, and the line is marked `AVON PATCH`.

Without it, ECDHE never completes in this configuration. ECC comes only from the p256-m driver, so `MBEDTLS_ECP_LIGHT` is not defined and `MBEDTLS_ECP_MAX_BYTES` is a dummy 1. The TLS premaster buffer is then 1 byte, and `psa_raw_key_agreement()` fails with `PSA_ERROR_BUFFER_TOO_SMALL`. The device logs `TLS: handshake failed, -0x7F80` right after `Perform PSA-based ECDH computation`. This was found on TFW1 hardware against httpbin.org. Libraries built before this date (including MM-01's) have the defect for both profiles A and B.

Keep this patch when updating Mbed TLS, unless the new release fixes the sizing.

## When to rebuild
Rebuild **both** libraries after changing any of these:
- the two config headers (for example to switch to profile A);
- `src/network/tls_cfg_guard.c` or `tls_cfg_guard.h`;
- the vendored Mbed TLS sources.

Run this from Git Bash:
```
./build_lib.sh            # Debug and Release
./build_lib.sh Debug      # one configuration
```
The script uses the Atmel Studio toolchain at its default install path. Set `ARM_GCC_BIN` to use a different one. A full build takes about 30 minutes per configuration. Then rebuild the app in Atmel Studio.

**Guard:** if the app and library configurations don't match (for example a Debug app with the Release library, or an RSA app with an ECDSA library), the link fails with `undefined reference to mm_tls_cfg_guard_debug_rsa_ecdsa` (or `_release_rsa_ecdsa`; `_rsa` = RSA suite only, no suffix = ECDSA suite only).
