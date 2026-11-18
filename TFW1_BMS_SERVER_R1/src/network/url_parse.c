/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: url_parse.c
* Description	: Endpoint parser. Accepted forms:
*				    https://host[:port][/path]     (USE_HTTPS = 1, default port 443)
*				    http://host[:port][/path]      (USE_HTTPS = 0, default port 80)
*				    host[:port][/path]             (scheme of the build)
*				    a.b.c.d:port                   (legacy form, still accepted)
*				  The scheme of the other build is rejected, so a production
*				  device can never be configured back to plain HTTP.
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "conf_https.h"
#include "url_parse.h"

#if USE_HTTPS
#define DEFAULT_PORT	443u
#define OK_SCHEME		"https://"
#define BAD_SCHEME		"http://"
#else
#define DEFAULT_PORT	80u
#define OK_SCHEME		"http://"
#define BAD_SCHEME		"https://"
#endif

int url_parse(const char *s, mm_url_t *u)
{
	const char *p = s;
	const char *slash;
	const char *host_end;
	const char *colon;
	unsigned long port = DEFAULT_PORT;
	size_t host_len;

	memset(u, 0, sizeof(*u));
	if (s == NULL)
	{
		return -1;
	}
	if (strncmp(p, BAD_SCHEME, strlen(BAD_SCHEME)) == 0)
	{
		return -1;						/* wrong scheme for this build */
	}
	if (strncmp(p, OK_SCHEME, strlen(OK_SCHEME)) == 0)
	{
		p += strlen(OK_SCHEME);
	}

	slash = strchr(p, '/');
	host_end = (slash != NULL) ? slash : (p + strlen(p));
	colon = memchr(p, ':', (size_t)(host_end - p));
	if (colon != NULL)
	{
		char *end = NULL;

		port = strtoul(colon + 1, &end, 10);
		if ((end != host_end) || (port == 0u) || (port > 65535u))
		{
			return -1;
		}
		host_end = colon;
	}

	host_len = (size_t)(host_end - p);
	if ((host_len == 0u) || (host_len >= sizeof(u->host)))
	{
		return -1;
	}
	memcpy(u->host, p, host_len);
	u->port = (uint16_t)port;

	if ((slash != NULL) && (slash[1] != '\0') && (strlen(slash) < sizeof(u->path)))
	{
		strcpy(u->path, slash);
	}
	else
	{
		strncpy(u->path, HTTP_API_PATH, sizeof(u->path) - 1u);
	}
	return 0;
}

void url_host_header(const mm_url_t *u, char *out, size_t n)
{
	if (u->port == DEFAULT_PORT)
	{
		snprintf(out, n, "%s", u->host);
	}
	else
	{
		snprintf(out, n, "%s:%u", u->host, (unsigned int)u->port);
	}
}
