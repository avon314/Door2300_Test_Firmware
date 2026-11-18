/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: tls_diag.c
* Description	: DEBUG BUILDS ONLY - diagnosis of "TLS: handshake failed, -0x7F80"
*				  (MBEDTLS_ERR_SSL_HW_ACCEL_FAILED), which Mbed TLS returns from the
*				  ECDHE client key exchange without the underlying PSA status.
*
*				  The Debug configuration links with
*				    -Wl,--wrap=psa_generate_key -Wl,--wrap=psa_export_public_key
*				    -Wl,--wrap=psa_raw_key_agreement -Wl,--wrap=psa_destroy_key
*				  so the library's calls land here; each wrapper calls the real
*				  function and prints its status. The prebuilt libmbedtls41.a is not
*				  modified. Remove the flags and this file once the cause is fixed.
*****************************************************************************/
#if defined(DEBUG)

#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mbedtls/build_info.h"
#include "psa/crypto.h"
#include "user_uart.h"

psa_status_t __real_psa_generate_key(const psa_key_attributes_t *attributes, mbedtls_svc_key_id_t *key);
psa_status_t __real_psa_export_public_key(mbedtls_svc_key_id_t key, uint8_t *data, size_t data_size,
										  size_t *data_length);
psa_status_t __real_psa_raw_key_agreement(psa_algorithm_t alg, mbedtls_svc_key_id_t private_key,
										  const uint8_t *peer_key, size_t peer_key_length,
										  uint8_t *output, size_t output_size, size_t *output_length);
psa_status_t __real_psa_destroy_key(mbedtls_svc_key_id_t key);

psa_status_t __wrap_psa_generate_key(const psa_key_attributes_t *attributes, mbedtls_svc_key_id_t *key);
psa_status_t __wrap_psa_export_public_key(mbedtls_svc_key_id_t key, uint8_t *data, size_t data_size,
										  size_t *data_length);
psa_status_t __wrap_psa_raw_key_agreement(psa_algorithm_t alg, mbedtls_svc_key_id_t private_key,
										  const uint8_t *peer_key, size_t peer_key_length,
										  uint8_t *output, size_t output_size, size_t *output_length);
psa_status_t __wrap_psa_destroy_key(mbedtls_svc_key_id_t key);

static void diag_num(const char *label, long v)
{
	Print_Message(label);
	if (v < 0)
	{
		Print_Message("-");
		v = -v;
	}
	Print_Number((int)v);
}

static void diag_status(const char *what, psa_status_t st)
{
	Print_Message("\nDIAG ");
	Print_Message(what);
	diag_num(" status=", (long)st);
}

psa_status_t __wrap_psa_generate_key(const psa_key_attributes_t *attributes, mbedtls_svc_key_id_t *key)
{
	psa_status_t st = __real_psa_generate_key(attributes, key);

	diag_status("generate_key", st);
	diag_num(" type=", (long)psa_get_key_type(attributes));	/* decimal; ECC key pair SECP_R1 = 28946 (0x7112) */
	diag_num(" bits=", (long)psa_get_key_bits(attributes));
	diag_num(" id=", (long)MBEDTLS_SVC_KEY_ID_GET_KEY_ID(*key));
	diag_num(" stack_free_words=", (long)uxTaskGetStackHighWaterMark(NULL));
	return st;
}

psa_status_t __wrap_psa_export_public_key(mbedtls_svc_key_id_t key, uint8_t *data, size_t data_size,
										  size_t *data_length)
{
	psa_status_t st = __real_psa_export_public_key(key, data, data_size, data_length);

	diag_status("export_public_key", st);
	diag_num(" size=", (long)data_size);
	diag_num(" len=", (long)*data_length);
	return st;
}

psa_status_t __wrap_psa_raw_key_agreement(psa_algorithm_t alg, mbedtls_svc_key_id_t private_key,
										  const uint8_t *peer_key, size_t peer_key_length,
										  uint8_t *output, size_t output_size, size_t *output_length)
{
	psa_status_t st = __real_psa_raw_key_agreement(alg, private_key, peer_key, peer_key_length,
												   output, output_size, output_length);

	diag_status("raw_key_agreement", st);
	diag_num(" peer_len=", (long)peer_key_length);
	diag_num(" peer[0]=", (long)((peer_key_length > 0u) ? peer_key[0] : -1));
	diag_num(" out_size=", (long)output_size);
	diag_num(" out_len=", (long)*output_length);
	return st;
}

psa_status_t __wrap_psa_destroy_key(mbedtls_svc_key_id_t key)
{
	psa_status_t st = __real_psa_destroy_key(key);

	if (st != PSA_SUCCESS)		/* called often (RSA verify etc.): report failures only */
	{
		diag_status("destroy_key", st);
		diag_num(" id=", (long)MBEDTLS_SVC_KEY_ID_GET_KEY_ID(key));
	}
	return st;
}

#endif /* DEBUG */
