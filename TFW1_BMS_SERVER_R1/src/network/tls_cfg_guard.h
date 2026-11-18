/*****************************************************************************
* Module Name	: tls_cfg_guard.h
* Description	: Link-time check that the app and libmbedtls41.a were built
*				  with the same Mbed TLS configuration (see tls_cfg_guard.c).
*				  Include only after the Mbed TLS config is in effect
*				  (e.g. after "mbedtls/build_info.h").
*****************************************************************************/
#ifndef TLS_CFG_GUARD_H_
#define TLS_CFG_GUARD_H_

/* The symbol name encodes the build configuration (Debug/Release) and the cipher
 * profile, so a library built for a different profile cannot be linked:
 *   mm_tls_cfg_guard_<debug|release>            ECDHE-ECDSA only (profile A)
 *   mm_tls_cfg_guard_<debug|release>_rsa        ECDHE-RSA only   (profile B)
 *   mm_tls_cfg_guard_<debug|release>_rsa_ecdsa  both suites      (2026-11-18) */
#if defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_DEBUG	1
#elif !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && !defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_DEBUG	0
#else
#error "Inconsistent MM-01 Mbed TLS configuration"
#endif

#if defined(MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED) && defined(MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED)
#if MM_TLS_CFG_DEBUG
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_debug_rsa_ecdsa
#else
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_release_rsa_ecdsa
#endif
#elif defined(MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED)
#if MM_TLS_CFG_DEBUG
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_debug_rsa
#else
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_release_rsa
#endif
#else
#if MM_TLS_CFG_DEBUG
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_debug
#else
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_release
#endif
#endif

extern const int MM_TLS_CFG_GUARD_SYMBOL;

#endif /* TLS_CFG_GUARD_H_ */
