/**
 * \file
 *
 * \brief Httpd server.
 *
 * Copyright (c) 2013-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/*****************************************************************************
* Module Name	: httpd.c
* Description	: TCP server on port 502 for the firmware-update handshake with
*				  the PC utility (Download_New_Firmware()).
*
* REVISION HISTORY
* 16/11/2026	: Ported from the lwIP raw API (NO_SYS = 1, callbacks) to a
*				  FreeRTOS task on the netconn API, as part of the HTTPS port
*				  (lwIP now runs threaded; raw-API calls are only allowed from
*				  the tcpip thread). Behaviour is unchanged: every received
*				  segment is copied (max MAX_BIN_BYTES), passed to
*				  Download_New_Firmware() and answered with gb_ack_buff.
*				  The unused raw-API HTTP page server code was removed.
*****************************************************************************/

#include <string.h>
#include "asf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/api.h"
#include "httpd.h"
#include "defination.h"

bool gb_connect_cmd_rec = 0;

extern U8 Frame_Len ;

/*****************************************************************************
* Function name	: static void Handle_Received_Frame(struct netconn *conn, struct netbuf *inbuf)
* Returns		: Nothing.
* Arguments		: conn  ---> Accepted connection.
*				: inbuf ---> Received data (one or more pbufs).
* Description	: Copies the received bytes (bounded), runs the firmware-update
*				  command handler and sends the acknowledgement frame.
*				  Same processing as the former raw-API tcp_server_recv().
*****************************************************************************/
static void Handle_Received_Frame(struct netconn *conn, struct netbuf *inbuf)
{
	static uint8_t gb_bin_buffer[MAX_BIN_BYTES + 1];	/* +1: NUL for the debug print */
	uint16_t gb_bin_byte_count = 0;

	memset(gb_bin_buffer, 0, sizeof(gb_bin_buffer));
	gb_bin_byte_count = netbuf_copy(inbuf, gb_bin_buffer, MAX_BIN_BYTES);	/* bounded copy over the pbuf chain */

	#if /*DEBUG_NEW_APP*/1
	Print_Message("\nReceived bytes so far : ");
	Print_Number(gb_bin_byte_count);
	Print_Message("\n******************************");
	Print_Message("\nReceived DATA : ");
	Print_Message((const char *)gb_bin_buffer);
	Print_Message("\n******************************");
	#endif

	Download_New_Firmware(gb_bin_buffer, gb_bin_byte_count);
	netconn_write(conn, gb_ack_buff, Frame_Len, NETCONN_COPY);

	#if /*DEBUG_NEW_APP*/1
	Print_Message("\nSending ACK bytes : ");
	Print_Message((const char *)gb_ack_buff);
	Print_Message("\n******************************");
	#endif

	if(gb_download_frame == true)
	{
		gb_connect_cmd_rec = true;
	}
}

/*****************************************************************************
* Function name	: void Firmware_Server_Task(void *pvParameters)
* Returns		: Nothing (task).
* Arguments		: pvParameters ---> Unused.
* Description	: Listens on port 502 and serves one client at a time.
*****************************************************************************/
void Firmware_Server_Task(void *pvParameters)
{
	struct netconn *conn;
	struct netconn *newconn;
	struct netbuf *inbuf;
	err_t err;

	UNUSED(pvParameters);

	conn = netconn_new(NETCONN_TCP);
	if (conn == NULL)
	{
		Print_Message("\nPort 502: netconn_new failed.");
		vTaskDelete(NULL);
	}
	if ((netconn_bind(conn, NULL, 502) != ERR_OK) || (netconn_listen(conn) != ERR_OK))
	{
		Print_Message("\nPort 502: bind/listen failed.");
		netconn_delete(conn);
		vTaskDelete(NULL);
	}

	for (;;)
	{
		err = netconn_accept(conn, &newconn);	/* blocks until a client connects */
		if (err == ERR_OK)
		{
			while (netconn_recv(newconn, &inbuf) == ERR_OK)
			{
				Handle_Received_Frame(newconn, inbuf);
				netbuf_delete(inbuf);
			}
			/* Client closed the connection, or an error occurred. */
			netconn_close(newconn);
			netconn_delete(newconn);
		}
		else
		{
			vTaskDelay(10);
		}
	}
}
