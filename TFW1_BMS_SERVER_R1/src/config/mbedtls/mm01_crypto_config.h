/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: mm01_crypto_config.h
* Description	: TF-PSA-Crypto 1.1 configuration for MM-01.
*				  Full replacement of tf-psa-crypto/include/psa/crypto_config.h,
*				  selected with -DTF_PSA_CRYPTO_CONFIG_FILE="mm01_crypto_config.h".
*				  Anything not defined here is OFF.
*
*				  Algorithms: ECDH + ECDSA on P-256 (p256-m driver), SHA-256,
*				  HMAC, TLS 1.2 PRF, AES-128-GCM, CTR_DRBG.
*				  Entropy: factory NV seed in internal flash only (no TRNG on SAM4E).
*****************************************************************************/
#ifndef MM01_CRYPTO_CONFIG_H
#define MM01_CRYPTO_CONFIG_H

/* ---- Core and platform ---- */
#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY                 /* calloc/free go to the static pool */
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME                       /* session-ticket lifetime, timeouts */
#define MBEDTLS_PLATFORM_MS_TIME_ALT            /* mbedtls_ms_time() in tls_platform.c */
/* MBEDTLS_HAVE_TIME_DATE off: no certificate dates are checked (VERIFY_NONE). */

/* ---- ECC: P-256 only, via the small p256-m driver ---- */
#define PSA_WANT_ALG_ECDH                       1
#define PSA_WANT_ALG_ECDSA                      1
#define PSA_WANT_ECC_SECP_R1_256                1
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY        1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC    1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT   1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_EXPORT   1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define MBEDTLS_PSA_P256M_DRIVER_ENABLED

/* ---- RSA server certificates (ECDHE-RSA, 2026-11-16) ----
 * KEY_PAIR_BASIC is required by Mbed TLS 4.1 for ECDHE-RSA (mbedtls_check_config.h),
 * for offering rsa_pkcs1_sha256 and for recognising sha256WithRSAEncryption certificates. */
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY        1
#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC    1
#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN          1

/* ---- Hash, MAC, PRF ---- */
#define PSA_WANT_ALG_SHA_256                    1
#define PSA_WANT_ALG_HMAC                       1
#define PSA_WANT_KEY_TYPE_HMAC                  1
#define PSA_WANT_ALG_TLS12_PRF                  1
#define MBEDTLS_SHA256_SMALLER

/* ---- Record protection ---- */
#define PSA_WANT_ALG_GCM                        1
#define PSA_WANT_KEY_TYPE_AES                   1
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_AES_FEWER_TABLES

/* ---- Random number generator: CTR_DRBG seeded only from the NV seed ---- */
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_NV_SEED
#define MBEDTLS_ENTROPY_NO_SOURCES_OK
#define MBEDTLS_PLATFORM_NV_SEED_ALT            /* mbedtls_platform_set_nv_seed() */

/* ---- Parsing support for certificates and keys ---- */
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_MD_C

/* ---- Key store without heap ---- */
#define MBEDTLS_PSA_STATIC_KEY_SLOTS
#define MBEDTLS_PSA_KEY_SLOT_COUNT              8
/* Without this, each static slot is sized for an RSA-4096 key PAIR (~2.4 KB x 8 = 19 KB RAM)
 * once RSA_KEY_PAIR_BASIC is enabled. The device only holds the server's RSA PUBLIC key
 * (RSA-4096 public DER = 526 B), the ECDHE P-256 key and AES/HMAC keys. */
#define MBEDTLS_PSA_STATIC_KEY_SLOT_BUFFER_SIZE 600

/* ---- Development builds only ---- */
#if defined(DEBUG)
#define MBEDTLS_MEMORY_DEBUG                    /* pool peak/current statistics */
#endif

#endif /* MM01_CRYPTO_CONFIG_H */
