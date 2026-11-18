/*****************************************************************************
* Module Name	: net_xport.h
* Description	: Transport used by the server client. The implementation is
*				  chosen at compile time by USE_HTTPS (conf_https.h):
*				  TLS over netconn, or plain netconn for lab builds.
*****************************************************************************/
#ifndef NET_XPORT_H_
#define NET_XPORT_H_

#include <stddef.h>
#include <stdint.h>

int  net_xport_open(const char *host, uint16_t port);		/* 0 = connected */
int  net_xport_write(const unsigned char *buf, int len);	/* bytes written or <0 */
int  net_xport_read(unsigned char *buf, size_t len);		/* >0 bytes, 0 = end (closed/timeout), <0 error */
void net_xport_close(void);

#endif /* NET_XPORT_H_ */
