/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: net_xport.c
* Description	: Transport for the server client.
*				  USE_HTTPS = 1 : wrappers over tls_client (TLS 1.2).
*				  USE_HTTPS = 0 : plain netconn, same behaviour as the original
*				                  client code (2 s receive timeout). Lab only.
*****************************************************************************/
#include "conf_https.h"
#include "net_xport.h"

#if USE_HTTPS

#include "mbedtls/build_info.h"
#include "mbedtls/ssl.h"
#include "tls_client.h"

int net_xport_open(const char *host, uint16_t port)
{
	int ret = tls_platform_init();

	if (ret != 0)
	{
		return ret;				/* no seed: fail closed, never fall back to plaintext */
	}
	return tls_client_connect(host, port);
}

int net_xport_write(const unsigned char *buf, int len)
{
	return tls_client_write(buf, (size_t)len);
}

int net_xport_read(unsigned char *buf, size_t len)
{
	int ret = tls_client_read(buf, len);

	return (ret == MBEDTLS_ERR_SSL_TIMEOUT) ? 0 : ret;	/* timeout ends the response */
}

void net_xport_close(void)
{
	tls_client_close();
}

#else /* ---------------- plain HTTP (lab builds) ---------------- */

#include "lwip/api.h"

static struct netconn	*s_conn;
static struct netbuf	*s_nb;
static u16_t			s_off;

int net_xport_open(const char *host, uint16_t port)
{
	ip_addr_t ip;

	s_nb = NULL;
	if (netconn_gethostbyname(host, &ip) != ERR_OK)		/* also accepts "a.b.c.d" */
	{
		return -1;
	}
	s_conn = netconn_new(NETCONN_TCP);
	if (s_conn == NULL)
	{
		return -1;
	}
	if (netconn_connect(s_conn, &ip, port) != ERR_OK)
	{
		net_xport_close();
		return -1;
	}
	netconn_set_recvtimeout(s_conn, 2000);				/* same 2 s as before */
	return 0;
}

int net_xport_write(const unsigned char *buf, int len)
{
	return (netconn_write(s_conn, buf, (size_t)len, NETCONN_COPY) == ERR_OK) ? len : -1;
}

int net_xport_read(unsigned char *buf, size_t len)
{
	u16_t avail;
	u16_t n;

	if (s_nb == NULL)
	{
		if (netconn_recv(s_conn, &s_nb) != ERR_OK)		/* timeout or closed: end of response */
		{
			s_nb = NULL;
			return 0;
		}
		s_off = 0;
	}
	avail = (u16_t)(netbuf_len(s_nb) - s_off);
	n = (len < avail) ? (u16_t)len : avail;
	netbuf_copy_partial(s_nb, buf, n, s_off);
	s_off = (u16_t)(s_off + n);
	if (s_off >= netbuf_len(s_nb))
	{
		netbuf_delete(s_nb);
		s_nb = NULL;
	}
	return (int)n;
}

void net_xport_close(void)
{
	if (s_nb != NULL)
	{
		netbuf_delete(s_nb);
		s_nb = NULL;
	}
	if (s_conn != NULL)
	{
		netconn_close(s_conn);
		netconn_delete(s_conn);
		s_conn = NULL;
	}
}

#endif /* USE_HTTPS */
