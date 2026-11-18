/*****************************************************************************
* Module Name	: url_parse.h
* Description	: Parses the configured server endpoint
*				  "[scheme://]host[:port][/path]" into its parts.
*****************************************************************************/
#ifndef URL_PARSE_H_
#define URL_PARSE_H_

#include <stdint.h>
#include <stddef.h>

typedef struct
{
	char		host[128];
	uint16_t	port;
	char		path[128];
} mm_url_t;

int  url_parse(const char *s, mm_url_t *u);							/* 0 = OK, -1 = invalid */
void url_host_header(const mm_url_t *u, char *out, size_t n);		/* "host" or "host:port" */

#endif /* URL_PARSE_H_ */
