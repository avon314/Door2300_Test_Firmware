/*****************************************************************************
* Module Name	: tls_platform.h
* Description	: Platform services for Mbed TLS on MM-01: millisecond time,
*				  factory NV seed (A/B slots in internal flash).
*****************************************************************************/
#ifndef TLS_PLATFORM_H_
#define TLS_PLATFORM_H_

#include <stddef.h>
#include "conf_https.h"

#if USE_HTTPS

#define MM_NV_SEED_LEN		32u		/* MBEDTLS_ENTROPY_BLOCK_SIZE with SHA-256 RNG hash */

int mm_nv_seed_present(void);
int mm_nv_seed_read(unsigned char *buf, size_t len);	/* Mbed TLS callback */
int mm_nv_seed_write(unsigned char *buf, size_t len);	/* Mbed TLS callback */

#endif /* USE_HTTPS */
#endif /* TLS_PLATFORM_H_ */
