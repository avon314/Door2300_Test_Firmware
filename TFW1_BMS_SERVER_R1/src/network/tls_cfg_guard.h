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
 * profile (RSA/ECDSA), so a library built for a different profile cannot be linked. */
#if defined(MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED)
#if defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_debug_rsa
#elif !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && !defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_release_rsa
#else
#error "Inconsistent MM-01 Mbed TLS configuration"
#endif
#else
#if defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_debug
#elif !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE) && !defined(MBEDTLS_MEMORY_DEBUG)
#define MM_TLS_CFG_GUARD_SYMBOL	mm_tls_cfg_guard_release
#else
#error "Inconsistent MM-01 Mbed TLS configuration"
#endif
#endif

extern const int MM_TLS_CFG_GUARD_SYMBOL;

#endif /* TLS_CFG_GUARD_H_ */
