/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: https_client.c
* Description	: Client task: every HTTPS_POLL_INTERVAL_MS it POSTs a small JSON
*				  status frame to HTTPS_SERVER_ENDPOINT (conf_https.h) and prints
*				  the JSON reply on the debug UART.
*				  Transport: net_xport (TLS 1.2 with USE_HTTPS = 1, plain HTTP
*				  for lab builds with USE_HTTPS = 0).
*				  Call sequence as in the HTTPS porting guide, section 7 step 7.
*
* Notes			: This is the ONLY task that calls TLS / crypto functions (guide
*				  section 3: "one task"), so Mbed TLS threading support stays off.
*				  Factory_Cmd_Process() (UART seed provisioning) runs here for the
*				  same reason.
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "asf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/netif.h"
#include "conf_https.h"
#include "user_uart.h"
#include "net_xport.h"
#include "url_parse.h"
#include "tls_client.h"
#include "factory_cmd.h"
#include "newlib_lock.h"
#include "https_client.h"

#define DEVICE_NAME		"TFW1_BMS_SERVER_R1"

extern struct netif gs_net_if;	/* ethernet.c */

/* Request and response buffer. The request must fit MBEDTLS_SSL_OUT_CONTENT_LEN
 * (2048) per record; tls_client_write() splits longer data anyway. */
static char s_http_buf[1024];
static char s_json[160];

#if MM_MEM_STATS
/*****************************************************************************
* Function name	: static void Mem_Stats_Print(void)
* Description	: Prints FreeRTOS heap, client-task stack, newlib heap and (HTTPS
*				  builds) TLS pool usage after every poll (guide 6.5 and 6.8).
*****************************************************************************/
static void Mem_Stats_Print(void)
{
	Print_Message("\nFreeRTOS heap free / size : ");
	Print_Number((int)xPortGetFreeHeapSize());
	Print_Message(" / ");
	Print_Number((int)configTOTAL_HEAP_SIZE);
	Print_Message("\nClient stack unused words : ");
	Print_Number((int)uxTaskGetStackHighWaterMark(NULL));
	Print_Message("\nnewlib heap top : ");
	Print_Number((int)(U32)mm_sbrk_top());
	#if USE_HTTPS
	tls_pool_stats_print();
	#endif
}
#endif

/*****************************************************************************
* Function name	: static int http_read_body(char *out, size_t out_sz)
* Returns		: Body length, or -1 if no complete header / no response.
* Description	: Reads the HTTP response through net_xport, finds the end of the
*				  headers ("\r\n\r\n") even across read boundaries, copies the body
*				  into out (bounded) and NUL-terminates it.
*				  From the guide, appendix A1. MM-01's xSemaphore handling is
*				  removed: this firmware has no data shared with other tasks here.
*****************************************************************************/
static int http_read_body(char *out, size_t out_sz)
{
	static unsigned char chunk[512];
	static const char eoh[4] = { '\r', '\n', '\r', '\n' };
	U8 match = 0;
	U8 in_body = 0;
	size_t total = 0;
	int n;
	int i;

	while ((n = net_xport_read(chunk, sizeof(chunk))) > 0)
	{
		wdt_restart(WDT);	// Restart the watch dog timer (no effect while it is disabled).
		for (i = 0; i < n; i++)
		{
			if (in_body == 0)
			{
				if (chunk[i] == (unsigned char)eoh[match])
				{
					match++;
				}
				else
				{
					match = (chunk[i] == '\r') ? 1 : 0;
				}
				if (match == 4)
				{
					in_body = 1;
				}
			}
			else if (total < (out_sz - 1u))
			{
				out[total++] = (char)chunk[i];
			}
			else
			{
				__NOP();	// Body larger than the buffer: the rest is dropped.
			}
		}
	}
	out[total] = '\0';
	return (in_body != 0) ? (int)total : -1;
}

/*****************************************************************************
* Function name	: static int Build_Request(const mm_url_t *ep, const char *host_hdr, U32 seq)
* Returns		: Request length, or -1 if it does not fit the buffer.
* Description	: Builds the JSON status frame and the HTTP POST request
*				  (same request format as MM-01).
*****************************************************************************/
static int Build_Request(const mm_url_t *ep, const char *host_hdr, U32 seq)
{
	int json_len;
	int req_len;

	json_len = snprintf(s_json, sizeof(s_json),
		"{\"device\":\"%s\",\"ip\":\"%u.%u.%u.%u\",\"seq\":%lu,\"uptime_s\":%lu}",
		DEVICE_NAME,
		(unsigned int)ip4_addr1(&gs_net_if.ip_addr), (unsigned int)ip4_addr2(&gs_net_if.ip_addr),
		(unsigned int)ip4_addr3(&gs_net_if.ip_addr), (unsigned int)ip4_addr4(&gs_net_if.ip_addr),
		(unsigned long)seq,
		(unsigned long)(xTaskGetTickCount() / configTICK_RATE_HZ));
	if ((json_len < 0) || (json_len >= (int)sizeof(s_json)))
	{
		return -1;
	}

	req_len = snprintf(s_http_buf, sizeof(s_http_buf),
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/json\r\n"
		"Authorization: token %s\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n\r\n"
		"%s",
		ep->path, host_hdr, HTTPS_AUTH_TOKEN, json_len, s_json);
	if ((req_len < 0) || (req_len >= (int)sizeof(s_http_buf)))
	{
		return -1;
	}
	return req_len;
}

/*****************************************************************************
* Function name	: void HTTPS_Client_Task(void *pvParameters)
* Returns		: Nothing (task).
* Arguments		: pvParameters ---> Unused.
* Description	: Parses the endpoint once, then every HTTPS_POLL_INTERVAL_MS:
*				  open (TCP + TLS handshake), POST, read the body, close.
*				  A failed poll is simply retried at the next interval.
*				  HTTPS never falls back to plain HTTP (fail closed).
*****************************************************************************/
void HTTPS_Client_Task(void *pvParameters)
{
	mm_url_t ep;
	char host_hdr[sizeof(ep.host) + 8];
	U8 ep_valid = 0;
	U32 seq = 0;
	portTickType last_poll;

	UNUSED(pvParameters);

	if (url_parse(HTTPS_SERVER_ENDPOINT, &ep) == 0)
	{
		ep_valid = 1;
		url_host_header(&ep, host_hdr, sizeof(host_hdr));
		Print_Message("\nServer : ");
		Print_Message(ep.host);
		Print_Message(":");
		Print_Number(ep.port);
		Print_Message(ep.path);
	}
	else
	{
		Print_Message("\nError in the configured server endpoint.");
		#if USE_HTTPS
		Print_Message(" (HTTPS build: http:// endpoints are rejected)");
		#else
		Print_Message(" (HTTP lab build: https:// endpoints are rejected)");
		#endif
	}

	/* First POST one interval after start-up (link negotiation, ARP). */
	last_poll = xTaskGetTickCount();

	for (;;)
	{
		Factory_Cmd_Process();	// UART factory commands (TLS seed) - same task that uses the crypto.

		if ((ep_valid == 1) && netif_is_up(&gs_net_if) &&
			((portTickType)(xTaskGetTickCount() - last_poll) >= (HTTPS_POLL_INTERVAL_MS / portTICK_RATE_MS)))
		{
			last_poll = xTaskGetTickCount();
			seq++;

			// Connect (TCP + TLS handshake in HTTPS builds). No lock is held here.
			if (net_xport_open(ep.host, ep.port) == 0)
			{
				int req_len = Build_Request(&ep, host_hdr, seq);

				if (req_len < 0)
				{
					Print_Message("\nHTTP request too long for the buffer.");
				}
				else if (net_xport_write((const unsigned char *)s_http_buf, req_len) != req_len)
				{
					Print_Message("\nFailed to send HTTP request.");
				}
				else
				{
					int body_len;

					Print_Message("\n========== Sent request ===========\n");
					Print_Message(s_http_buf);

					// The response is received into the same buffer.
					body_len = http_read_body(s_http_buf, sizeof(s_http_buf));
					if (body_len >= 0)
					{
						int i;

						Print_Message("\n\nreceived length of the body is : ");
						Print_Number(body_len);
						Print_Message("\nReceived Data from the server is :-\n");
						for (i = 0; i < body_len; i++)
						{
							UART_Debug_PutChar((uint8_t)s_http_buf[i]);
						}
						Print_Message("\n---------- Complete reception ------------\n");
					}
					else
					{
						Print_Message("\nNo complete HTTP response from the server.");
					}
				}

				// Close the connection (TLS close_notify + TCP close in HTTPS builds).
				net_xport_close();
			}
			else
			{
				Print_Message("\nFailed to connect to server.");
			}

			#if MM_MEM_STATS
			Mem_Stats_Print();
			#endif
		}
		vTaskDelay(5);
	}
}
