/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: factory_cmd.c
* Description	: Factory commands on the debug UART (115200, line ending CR or LF):
*				    SEED <64 hex>   write the 32-byte TLS entropy seed (only if none)
*				    SEED?           report SEED:PRESENT / SEED:MISSING
*				  Replies: SEED:OK, SEED:REFUSED, SEED:BAD_HEX, SEED:WRITE_FAIL.
*				  The seed must come from the factory PC's OS random generator and
*				  be unique per device. Not available over the network (port 502).
*
* Notes			: The UART also carries the RTC time-set protocol; lines that are
*				  not factory commands are ignored here.
*****************************************************************************/
#include <string.h>
#include "conf_https.h"
#include "factory_cmd.h"

#if USE_HTTPS

#include "user_uart.h"
#include "tls_platform.h"

#define LINE_MAX	80u

static volatile char	s_line[LINE_MAX];
static volatile U8		s_len;
static volatile U8		s_ready;

void Factory_Cmd_RxByte(U8 c)
{
	if (s_ready)
	{
		return;							/* previous line not processed yet */
	}
	if ((c == '\r') || (c == '\n'))
	{
		if (s_len > 0u)
		{
			s_line[s_len] = '\0';
			s_ready = 1u;
		}
		return;
	}
	if (s_len < (LINE_MAX - 1u))
	{
		s_line[s_len++] = (char)c;
	}
	else
	{
		s_len = 0u;						/* overflow: drop the line */
	}
}

static int hex_nibble(char c)
{
	if ((c >= '0') && (c <= '9')) { return c - '0'; }
	if ((c >= 'a') && (c <= 'f')) { return c - 'a' + 10; }
	if ((c >= 'A') && (c <= 'F')) { return c - 'A' + 10; }
	return -1;
}

void Factory_Cmd_Process(void)
{
	char line[LINE_MAX];
	unsigned char seed[MM_NV_SEED_LEN];
	unsigned int i;

	if (!s_ready)
	{
		return;
	}
	for (i = 0u; i < LINE_MAX; i++)
	{
		line[i] = s_line[i];
	}
	line[LINE_MAX - 1u] = '\0';
	s_len = 0u;
	s_ready = 0u;

	if (strcmp(line, "SEED?") == 0)
	{
		Print_Message(mm_nv_seed_present() ? "\nSEED:PRESENT" : "\nSEED:MISSING");
	}
	else if ((strncmp(line, "SEED ", 5) == 0) && (strlen(line) == (5u + (2u * MM_NV_SEED_LEN))))
	{
		if (mm_nv_seed_present())
		{
			Print_Message("\nSEED:REFUSED");
		}
		else
		{
			int bad = 0;

			for (i = 0u; i < MM_NV_SEED_LEN; i++)
			{
				int hi = hex_nibble(line[5u + (2u * i)]);
				int lo = hex_nibble(line[6u + (2u * i)]);

				if ((hi < 0) || (lo < 0))
				{
					bad = 1;
					break;
				}
				seed[i] = (unsigned char)((hi << 4) | lo);
			}
			if (bad)
			{
				Print_Message("\nSEED:BAD_HEX");
			}
			else
			{
				Print_Message((mm_nv_seed_write(seed, sizeof(seed)) == (int)sizeof(seed))
				              ? "\nSEED:OK" : "\nSEED:WRITE_FAIL");
			}
			memset(seed, 0, sizeof(seed));
		}
	}
	else
	{
		/* not a factory command */
	}
	memset(line, 0, sizeof(line));
}

#else /* plain-HTTP lab build: no seed needed */

void Factory_Cmd_RxByte(U8 c)
{
	(void)c;
}

void Factory_Cmd_Process(void)
{
}

#endif /* USE_HTTPS */
