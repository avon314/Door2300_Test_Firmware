/*****************************************************************************
* Module Name	: tls_client.h
* Description	: TLS 1.2 client over lwIP netconn (Mbed TLS 4.1).
*				  One connection at a time; all calls from the client task only.
*****************************************************************************/
#ifndef TLS_CLIENT_H_
#define TLS_CLIENT_H_

#include <stddef.h>
#include <stdint.h>
#include "conf_https.h"

#if USE_HTTPS

#define TLS_ERR_NET			(-0x004E)	/* same value as MBEDTLS_ERR_NET_SEND_FAILED (NET_C not built) */
#define TLS_ERR_NO_SEED		(-0x7F01)	/* no factory seed: HTTPS refused (fail closed) */
#define TLS_ERR_INIT		(-0x7F02)

int  tls_platform_init(void);								/* idempotent; 0 = ready */
int  tls_client_connect(const char *host, uint16_t port);	/* 0 = handshake done */
int  tls_client_write(const unsigned char *buf, size_t len);/* bytes written or <0 */
int  tls_client_read(unsigned char *buf, size_t len);		/* >0 bytes, 0 = closed, <0 error */
void tls_client_close(void);
void tls_pool_stats_print(void);							/* MBEDTLS_MEMORY_DEBUG builds */

#endif /* USE_HTTPS */
#endif /* TLS_CLIENT_H_ */
