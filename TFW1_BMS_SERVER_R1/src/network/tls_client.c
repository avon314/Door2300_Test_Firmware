/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: tls_client.c
* Description	: TLS 1.2 client for the server POST, on Mbed TLS 4.1 over the
*				  lwIP netconn API.
*				  Profile: ECDHE-RSA-AES128-GCM-SHA256 (RSA server certificates),
*				  ECDHE on P-256, max fragment length 2048 requested, 16 KB receive
*				  buffer, session tickets for resumption.
*				  Server authentication: MBEDTLS_SSL_VERIFY_NONE (decision
*				  2026-11-06). Traffic is encrypted, the server identity is NOT
*				  verified - see docs/plans (Risk 0).
*
* Notes			: Single connection, used only from Send_Data_Client_To_Server.
*				  All Mbed TLS memory comes from a dedicated static pool.
*****************************************************************************/
#include "conf_https.h"

#if USE_HTTPS

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/api.h"
#include "psa/crypto.h"
#include "mbedtls/build_info.h"
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "user_uart.h"
#include "tls_client.h"
#include "tls_platform.h"
#include "tls_cfg_guard.h"

#define TLS_POOL_SIZE			(40960u)	/* 16 KB receive buffer + RSA chain/verify (2026-11-16) */
#define TLS_READ_TIMEOUT_MS		(5000u)

static unsigned char		s_pool[TLS_POOL_SIZE];
static mbedtls_ssl_config	s_conf;
static mbedtls_ssl_context	s_ssl;
static mbedtls_ssl_session	s_session;			/* kept between polls */
static int					s_have_session;
static int					s_ssl_active;		/* s_ssl initialised */
static int					s_handshake_done;
static int					s_ready;			/* tls_platform_init() done */

static struct
{
	struct netconn	*conn;
	struct netbuf	*nb;						/* partly consumed receive buffer */
	u16_t			off;
	portTickType	t_connect;					/* tick of TCP connect (diagnostics) */
} s_io;

/* ECDSA first (smaller, faster certificates); RSA for servers with RSA certificates. */
static const int		s_suites[] = { MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
									   MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, 0 };
static const uint16_t	s_groups[] = { MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,
									   MBEDTLS_SSL_IANA_TLS_GROUP_NONE };

#if defined(MBEDTLS_DEBUG_C)
#include "mbedtls/debug.h"

/* Mbed TLS debug level for Debug builds: 1 = errors, 2 = handshake steps,
 * 3 = negotiated values (cipher suite, curve, extensions). Hex dumps are filtered out.
 * TFW1: default 1 (MM-01: 3). At 9600 baud the level-3 trace makes a full handshake
 * take 10-20 s, longer than public servers allow; set MM_TLS_DEBUG_LEVEL=3 in the
 * project symbols only for a step-by-step trace against a local test server. */
#ifndef MM_TLS_DEBUG_LEVEL
#define MM_TLS_DEBUG_LEVEL		1
#endif

static int is_hex_dump_line(const char *s)
{
	/* Mbed TLS dump lines look like "0000:  16 03 03 ..." or "dumping 'xxx' (n bytes)".
	 * With this newlib printf (no %zx support) the offset prints as "000zx:", so
	 * those lines are matched here too - otherwise the hex dumps slow the handshake
	 * at 9600 baud past the server's handshake timeout. */
	int i;

	if ((strncmp(s, "dumping", 7) == 0) || (strncmp(s, "000zx:", 6) == 0))
	{
		return 1;
	}
	for (i = 0; i < 4; i++)
	{
		char c = s[i];
		if (!(((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'))))
		{
			return 0;
		}
	}
	return s[4] == ':';
}

static void tls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	const char *base = file;
	const char *p;

	(void)ctx;
	(void)level;
	if (is_hex_dump_line(str))
	{
		return;
	}
	for (p = file; *p != '\0'; p++)
	{
		if ((*p == '/') || (*p == '\\'))
		{
			base = p + 1;
		}
	}
	Print_Message("\nTLSDBG ");
	Print_Message(base);
	Print_Message(":");
	Print_Number(line);
	Print_Message(" ");
	Print_Message(str);				/* str ends with '\n' */
}
#endif

static void print_err(const char *what, int ret)
{
	unsigned int code = (unsigned int)(-ret);

	Print_Message("\nTLS: ");
	Print_Message(what);
	Print_Message(" failed, -0x");
	Print_ASCII_HEX((int)((code >> 8) & 0xFFu));
	Print_ASCII_HEX((int)(code & 0xFFu));
}

/* ---------------------------------------------------------------------------
 * BIO callbacks: Mbed TLS <-> lwIP netconn
 * ------------------------------------------------------------------------- */
static int bio_send(void *ctx, const unsigned char *buf, size_t len)
{
	err_t err;

	(void)ctx;
	err = netconn_write(s_io.conn, buf, len, NETCONN_COPY);
	if (err != ERR_OK)
	{
		void *pending = NULL;

		/* lwIP 1.4.1: -1 ERR_MEM, -3 ERR_TIMEOUT, -10 ERR_ABRT, -11 ERR_RST (peer reset), -12 ERR_CLSD */
		Print_Message("\nTLS: netconn_write err = -");
		Print_Number((int)(-err));
		Print_Message(", ms since TCP connect = ");
		Print_Number((int)((xTaskGetTickCount() - s_io.t_connect) * portTICK_RATE_MS));

		/* Diagnostics: show what the server sent before the reset, if anything is still
		 * queued (netconn_recv refuses to read after a fatal error, so peek the mailbox).
		 * A TLS alert record starts 15 03 03 00 02 <level> <description>. */
		if ((s_io.conn != NULL) && sys_mbox_valid(&s_io.conn->recvmbox) &&
			(sys_arch_mbox_tryfetch(&s_io.conn->recvmbox, &pending) != SYS_MBOX_EMPTY) &&
			(pending != NULL))
		{
			struct pbuf *p = (struct pbuf *)pending;
			unsigned char b[8];
			u16_t n = pbuf_copy_partial(p, b, sizeof(b), 0);
			u16_t i;

			Print_Message("\nTLS: server sent before reset (len ");
			Print_Number((int)p->tot_len);
			Print_Message("): ");
			for (i = 0; i < n; i++)
			{
				Print_ASCII_HEX((int)b[i]);
				Print_Message(" ");
			}
			pbuf_free(p);
		}
		else
		{
			Print_Message("\nTLS: nothing received from the server before the reset");
		}
		return TLS_ERR_NET;
	}
	return (int)len;
}

static int bio_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout_ms)
{
	u16_t avail;
	u16_t n;
	(void)ctx;

	if (s_io.nb == NULL)
	{
		err_t err;

		netconn_set_recvtimeout(s_io.conn, (int)timeout_ms);	/* 0 = block in both APIs */
		err = netconn_recv(s_io.conn, &s_io.nb);
		if (err == ERR_TIMEOUT)
		{
			s_io.nb = NULL;
			return MBEDTLS_ERR_SSL_TIMEOUT;
		}
		if (err == ERR_CLSD)
		{
			s_io.nb = NULL;
			return 0;									/* peer closed: EOF */
		}
		if (err != ERR_OK)
		{
			s_io.nb = NULL;
			return TLS_ERR_NET;
		}
		s_io.off = 0;
	}

	avail = (u16_t)(netbuf_len(s_io.nb) - s_io.off);
	n = (len < avail) ? (u16_t)len : avail;
	netbuf_copy_partial(s_io.nb, buf, n, s_io.off);
	s_io.off = (u16_t)(s_io.off + n);

	if (s_io.off >= netbuf_len(s_io.nb))
	{
		netbuf_delete(s_io.nb);
		s_io.nb = NULL;
	}
	return (int)n;
}

/* ---------------------------------------------------------------------------
 * One-time initialisation (retried every poll until a seed exists)
 * ------------------------------------------------------------------------- */
#if defined(MBEDTLS_DEBUG_C)
/* ---------------------------------------------------------------------------
 * Debug builds: one-shot check of the PSA primitives used by the ECDHE client
 * key exchange (diagnosis of -0x7F80 MBEDTLS_ERR_SSL_HW_ACCEL_FAILED, which
 * hides the PSA status). Prints "PSA <step> = <status>", 0 = PSA_SUCCESS.
 * ------------------------------------------------------------------------- */
static void print_status(const char *what, psa_status_t st)
{
	Print_Message("\nPSA ");
	Print_Message(what);
	Print_Message(" = ");
	if (st < 0)
	{
		Print_Message("-");
		Print_Number((int)(-st));
	}
	else
	{
		Print_Number((int)st);
	}
}

static void psa_selftest(void)
{
	psa_key_attributes_t attr = psa_key_attributes_init();
	mbedtls_svc_key_id_t ka = MBEDTLS_SVC_KEY_ID_INIT;
	mbedtls_svc_key_id_t kb = MBEDTLS_SVC_KEY_ID_INIT;
	uint8_t buf[65];
	uint8_t secret[32];
	size_t len = 0;
	size_t slen = 0;
	psa_status_t st;

	Print_Message("\nPSA self-test (ECDHE P-256):");
	st = psa_generate_random(buf, 32);
	print_status("generate_random", st);

	psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE);
	psa_set_key_algorithm(&attr, PSA_ALG_ECDH);
	psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_bits(&attr, 256);

	st = psa_generate_key(&attr, &ka);
	print_status("generate_key A", st);
	st = psa_generate_key(&attr, &kb);
	print_status("generate_key B", st);
	st = psa_export_public_key(kb, buf, sizeof(buf), &len);
	print_status("export_public_key B", st);
	Print_Message(" len ");
	Print_Number((int)len);
	st = psa_raw_key_agreement(PSA_ALG_ECDH, ka, buf, len, secret, sizeof(secret), &slen);
	print_status("raw_key_agreement", st);
	Print_Message(" len ");
	Print_Number((int)slen);

	(void)psa_destroy_key(ka);
	(void)psa_destroy_key(kb);
	tls_pool_stats_print();
}
#endif

int tls_platform_init(void)
{
	if (MM_TLS_CFG_GUARD_SYMBOL != 1)		/* real reference: link error if app/library configs differ */
	{
		return TLS_ERR_INIT;
	}
	if (s_ready)
	{
		return 0;
	}
	if (!mm_nv_seed_present())
	{
		Print_Message("\nTLS: no entropy seed provisioned - HTTPS disabled (use UART: SEED <64 hex>)");
		return TLS_ERR_NO_SEED;
	}

	mbedtls_memory_buffer_alloc_init(s_pool, sizeof(s_pool));
	mbedtls_platform_set_nv_seed(mm_nv_seed_read, mm_nv_seed_write);

	if (psa_crypto_init() != PSA_SUCCESS)
	{
		Print_Message("\nTLS: psa_crypto_init failed");
		return TLS_ERR_INIT;
	}
#if defined(MBEDTLS_DEBUG_C)
	psa_selftest();		/* Debug builds: ECDHE primitives check, see above */
#endif

	mbedtls_ssl_config_init(&s_conf);
	if (mbedtls_ssl_config_defaults(&s_conf, MBEDTLS_SSL_IS_CLIENT,
	                                MBEDTLS_SSL_TRANSPORT_STREAM,
	                                MBEDTLS_SSL_PRESET_DEFAULT) != 0)
	{
		return TLS_ERR_INIT;
	}
	mbedtls_ssl_conf_min_tls_version(&s_conf, MBEDTLS_SSL_VERSION_TLS1_2);
	mbedtls_ssl_conf_max_tls_version(&s_conf, MBEDTLS_SSL_VERSION_TLS1_2);
	mbedtls_ssl_conf_authmode(&s_conf, MBEDTLS_SSL_VERIFY_NONE);	/* decision 2026-11-06 */
	mbedtls_ssl_conf_ciphersuites(&s_conf, s_suites);
	mbedtls_ssl_conf_groups(&s_conf, s_groups);
	/* MFL may not exceed min(IN, OUT) content length = 2048; 4096 was silently rejected before. */
	if (mbedtls_ssl_conf_max_frag_len(&s_conf, MBEDTLS_SSL_MAX_FRAG_LEN_2048) != 0)
	{
		Print_Message("\nTLS: max_fragment_length setting rejected");
	}
	mbedtls_ssl_conf_session_tickets(&s_conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
	mbedtls_ssl_conf_read_timeout(&s_conf, TLS_READ_TIMEOUT_MS);
#if defined(MBEDTLS_DEBUG_C)
	mbedtls_ssl_conf_dbg(&s_conf, tls_debug, NULL);		/* Debug builds only */
	mbedtls_debug_set_threshold(MM_TLS_DEBUG_LEVEL);
#endif

	mbedtls_ssl_session_init(&s_session);
	s_ready = 1;
	Print_Message("\nTLS: ready (TLS 1.2, VERIFY_NONE)");
	return 0;
}

/* ---------------------------------------------------------------------------
 * Development builds: log the server certificate SHA-256 fingerprint
 * ------------------------------------------------------------------------- */
#if defined(MM_TLS_LOG_FINGERPRINT) && defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
static void log_peer_fingerprint(void)
{
	const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&s_ssl);	/* NULL when resumed */
	uint8_t hash[32];
	size_t hash_len = 0;
	size_t i;

	if (crt == NULL)
	{
		return;
	}
	if (psa_hash_compute(PSA_ALG_SHA_256, crt->raw.p, crt->raw.len,
	                     hash, sizeof(hash), &hash_len) != PSA_SUCCESS)
	{
		return;
	}
	Print_Message("\nTLS: server cert SHA-256 ");
	for (i = 0; i < hash_len; i++)
	{
		Print_ASCII_HEX((int)hash[i]);
	}
}
#else
#define log_peer_fingerprint()	((void)0)
#endif

/* ---------------------------------------------------------------------------
 * Per-poll connection
 * ------------------------------------------------------------------------- */
int tls_client_connect(const char *host, uint16_t port)
{
	ip_addr_t ip;
	portTickType t0;
	int ret;

	s_handshake_done = 0;
	s_io.nb = NULL;

	if (netconn_gethostbyname(host, &ip) != ERR_OK)
	{
		Print_Message("\nTLS: DNS lookup failed");
		return TLS_ERR_NET;
	}
	s_io.conn = netconn_new(NETCONN_TCP);
	if (s_io.conn == NULL)
	{
		return TLS_ERR_NET;
	}
	if (netconn_connect(s_io.conn, &ip, port) != ERR_OK)
	{
		ret = TLS_ERR_NET;
		goto fail;
	}
	s_io.t_connect = xTaskGetTickCount();

	mbedtls_ssl_init(&s_ssl);
	s_ssl_active = 1;
	if ((ret = mbedtls_ssl_setup(&s_ssl, &s_conf)) != 0)
	{
		print_err("setup", ret);
		goto fail;
	}
	if ((ret = mbedtls_ssl_set_hostname(&s_ssl, host)) != 0)	/* SNI only, not verified */
	{
		print_err("set_hostname", ret);
		goto fail;
	}
	if (s_have_session)
	{
		(void)mbedtls_ssl_set_session(&s_ssl, &s_session);	/* rejected ticket -> full handshake */
	}
	mbedtls_ssl_set_bio(&s_ssl, NULL, bio_send, NULL, bio_recv_timeout);

	t0 = xTaskGetTickCount();
	while ((ret = mbedtls_ssl_handshake(&s_ssl)) != 0)
	{
		if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
		{
			s_have_session = 0;								/* drop a possibly stale ticket */
			print_err("handshake", ret);
			tls_pool_stats_print();		/* MBEDTLS_MEMORY_DEBUG (Debug) builds: pool state at the failure */
			goto fail;
		}
	}
	s_handshake_done = 1;

	Print_Message("\nTLS: handshake ms = ");
	Print_Number((int)((xTaskGetTickCount() - t0) * portTICK_RATE_MS));
	log_peer_fingerprint();

	/* Keep the session (ticket) for a resumed handshake on the next poll. */
	mbedtls_ssl_session_free(&s_session);
	mbedtls_ssl_session_init(&s_session);
	s_have_session = (mbedtls_ssl_get_session(&s_ssl, &s_session) == 0);
	return 0;

fail:
	tls_client_close();
	return ret;
}

int tls_client_write(const unsigned char *buf, size_t len)
{
	size_t done = 0;

	while (done < len)
	{
		int ret = mbedtls_ssl_write(&s_ssl, buf + done, len - done);

		if (ret > 0)
		{
			done += (size_t)ret;
		}
		else if ((ret != MBEDTLS_ERR_SSL_WANT_WRITE) && (ret != MBEDTLS_ERR_SSL_WANT_READ))
		{
			print_err("write", ret);
			return ret;
		}
	}
	return (int)done;
}

int tls_client_read(unsigned char *buf, size_t len)
{
	for (;;)
	{
		int ret = mbedtls_ssl_read(&s_ssl, buf, len);

		if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE))
		{
			continue;
		}
		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
		{
			return 0;
		}
		return ret;		/* >0 data, 0 EOF, <0 error (incl. MBEDTLS_ERR_SSL_TIMEOUT) */
	}
}

void tls_client_close(void)
{
	if (s_handshake_done)
	{
		(void)mbedtls_ssl_close_notify(&s_ssl);
	}
	if (s_ssl_active)
	{
		mbedtls_ssl_free(&s_ssl);				/* per-connection memory back to the pool */
		s_ssl_active = 0;
	}
	s_handshake_done = 0;

	if (s_io.nb != NULL)
	{
		netbuf_delete(s_io.nb);
		s_io.nb = NULL;
	}
	if (s_io.conn != NULL)
	{
		netconn_close(s_io.conn);
		netconn_delete(s_io.conn);
		s_io.conn = NULL;
	}
}

void tls_pool_stats_print(void)
{
#if defined(MBEDTLS_MEMORY_DEBUG)
	size_t max_used, max_blocks, cur_used, cur_blocks;

	mbedtls_memory_buffer_alloc_max_get(&max_used, &max_blocks);
	mbedtls_memory_buffer_alloc_cur_get(&cur_used, &cur_blocks);
	Print_Message("\nTLS pool used/peak/size : ");
	Print_Number((int)cur_used);
	Print_Message(" / ");
	Print_Number((int)max_used);
	Print_Message(" / ");
	Print_Number((int)TLS_POOL_SIZE);
#endif
}

#endif /* USE_HTTPS */
