/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: mm01_mbedtls_config.h
* Description	: Mbed TLS 4.1 TLS/X.509 configuration for MM-01.
*				  Full replacement of include/mbedtls/mbedtls_config.h, selected with
*				  -DMBEDTLS_CONFIG_FILE="mm01_mbedtls_config.h". Anything not defined
*				  here is OFF.
*
*				  Profile: TLS 1.2 client only, one cipher suite
*				  (ECDHE-RSA-AES128-GCM-SHA256, RSA server certificates - changed
*				  2026-11-16 from ECDHE-ECDSA for public test servers), ECDHE on
*				  P-256, server not
*				  authenticated (MBEDTLS_SSL_VERIFY_NONE, decision 2026-11-06).
*
* Notes			: The app and the mbedtls41 library MUST be built with the same
*				  configuration (Debug with Debug, Release with Release), because
*				  the DEBUG-only options below change structure layouts. A mismatch
*				  is caught at link time by tls_cfg_guard.c.
*****************************************************************************/
#ifndef MM01_MBEDTLS_CONFIG_H
#define MM01_MBEDTLS_CONFIG_H

/* ---- Role and protocol ---- */
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED   /* 2026-11-16: RSA server certs (public test servers) */

/* ---- Certificates: needed to read the server's P-256 public key ---- */
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_REMOVE_INFO

/* ---- Extensions ---- */
#define MBEDTLS_SSL_SERVER_NAME_INDICATION      /* SNI: server picks virtual host/cert */
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET      /* RFC 7627 */
#define MBEDTLS_SSL_SESSION_TICKETS             /* resumption: no ECC on later polls */
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH         /* ask the server for 4 KB records */

/* ---- Record buffers (RAM) ---- */
#define MBEDTLS_SSL_IN_CONTENT_LEN              16384   /* RSA chains are 4-5 KB and servers ignore MFL */
#define MBEDTLS_SSL_OUT_CONTENT_LEN             2048

/* ---- Only this suite is compiled in ---- */
#define MBEDTLS_SSL_CIPHERSUITES                MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256

/* ---- Development builds only ---- */
#if defined(DEBUG)
#define MBEDTLS_DEBUG_C
#define MBEDTLS_ERROR_C
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE       /* needed to log the server cert fingerprint */
#define MM_TLS_LOG_FINGERPRINT
#endif

#endif /* MM01_MBEDTLS_CONFIG_H */
