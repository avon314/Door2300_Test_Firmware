# Vendored Mbed TLS

| Item | Value |
|---|---|
| Release | Mbed TLS **4.1.1** (LTS), with TF-PSA-Crypto 1.1.1 included |
| Published | 2026-07-07 |
| Source | https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-4.1.1/mbedtls-4.1.1.tar.bz2 |
| SHA-256 (tarball) | `3359a349e23db3d5536fcee032ae7b2ecbfc08972fab643089b5cbf2a375c98c` (matches the published `mbedtls-4.1.1-sha256sum.txt`) |
| License | Apache-2.0 OR GPL-2.0-or-later (see `LICENSE`) |

## Kept unmodified
- `include/`, `library/`: TLS and X.509. The generated files (`error.c`, `ssl_debug_helpers_generated.c`, `*_config_check_*.h`) come from the release tarball.
- `tf-psa-crypto/include`, `core`, `dispatch`, `extras`, `platform`, `utilities`, `drivers/builtin`, `drivers/p256-m`.
- `tf-psa-crypto/drivers/everest/include` and `tf-psa-crypto/drivers/pqcp/include` + `src`: headers only. They are on the driver include path but are not compiled.

## Removed
- `tests/`, `programs/`, `docs/`, `doxygen/`, `framework/`, `scripts/`, `configs/`, `cmake/`, `pkgconfig/`, `ChangeLog.d/`, `3rdparty/`. These are not needed to build the library.
- `tf-psa-crypto/drivers/everest/library` (X25519, disabled) and `tf-psa-crypto/drivers/pqcp/mldsa-native` (ML-DSA, disabled, 22 MB).
- Every `CMakeLists.txt`, `*.cmake`, `Makefile` and `Makefile.inc`. The library is built by `../mbedtls41/mbedtls41.cproj`.

## Configuration
These options come from the MM-01 config headers, not from the defaults in this tree:
- `-DMBEDTLS_CONFIG_FILE="mm01_mbedtls_config.h"`
- `-DTF_PSA_CRYPTO_CONFIG_FILE="mm01_crypto_config.h"`

Both headers are in `FW1_MM-01_R1/src/config/mbedtls/`.

## Updating
1. Download the new tarball and check it against the published SHA-256.
2. Repeat the pruning above.
3. Update this file.
4. Rebuild `mbedtls41` and fix any config-check errors.
