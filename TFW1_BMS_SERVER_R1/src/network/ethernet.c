/**
 * \file
 *
 * \brief Ethernet management for the lwIP Raw HTTP basic example.
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
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

#include <string.h>
#include "board.h"
#include "ethernet.h"
#if SAM3XA
# include "ethernet_phy.h"
# include "emac.h"
# include "netif/ethernetif.h"
#elif SAM4E
# include "ethernet_phy.h"
# include "gmac.h"
# include "netif/sam4e_gmac.h"
#elif SAM4S
# include "ksz8851snl.h"
# include "ksz8851snl_reg.h"
# include "netif/sam_spi_ksz8851snl.h"
#elif SAMD20
# include "ksz8851snl.h"
# include "ksz8851snl_reg.h"
# include "netif/sam0_spi_ksz8851snl.h"
#elif (SAMV71)
# include "ethernet_phy.h"
# include "gmac.h"
# include "netif/samv71_gmac.h"
#elif (SAME70)
# include "ethernet_phy.h"
# include "gmac.h"
# include "netif/same70_gmac.h"
#else
# error Unsupported chip type
#endif
#include "timer_mgt.h"
/* lwIP includes */
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/ip_frag.h"
#include "netif/sam4e_gmac.h"
#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
# include "netif/loopif.h"
#else
# include "lwip/inet.h"
# include "lwip/tcp_impl.h"
#endif
#include "netif/etharp.h"

#include "config_eth_ip_addr.h"
#include "user_uart.h"


/* Global variable containing MAC configuration (hw addr, IP, GW, etc). */
struct netif gs_net_if;



Ip_Addr IpAddr;     // Declare the structure variable of Ip_Addr to store the ip address.
char Port_Num[6];   // Declare the variable to store the port number.
Sbnt_Msk SbntMsk;   // Declare the structure variable of Sbnt_Msk to store the subnet mask.
Dflt_Gtwy DfltGtwy; // Declare the structure variable of Dflt_Gtwy to store the default gateway.

char gb_dev_ipAddress[12] = "abc";
/**
 * \brief Configure network interface driver.
 */
struct ip_addr x_ip_addr, x_net_mask, x_gateway;

static void ethernet_configure_interface(void)
{

	extern err_t ethernetif_init(struct netif *netif);

#if defined(DHCP_USED)
		/* DHCP mode. */
	x_ip_addr.addr = 0;
	x_net_mask.addr = 0;
#else

	/* Fixed IP mode. */
	/* Default ip addr */
	IP4_ADDR(&x_ip_addr, ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1,
   			ETHERNET_CONF_IPADDR2, ETHERNET_CONF_IPADDR3);

   	/* Default subnet mask */
   	IP4_ADDR(&x_net_mask, ETHERNET_CONF_NET_MASK0, ETHERNET_CONF_NET_MASK1,
   			ETHERNET_CONF_NET_MASK2, ETHERNET_CONF_NET_MASK3);

   	/* Default gateway addr */
   	IP4_ADDR(&x_gateway, ETHERNET_CONF_GATEWAY_ADDR0,
   			ETHERNET_CONF_GATEWAY_ADDR1,
   			ETHERNET_CONF_GATEWAY_ADDR2,
   			ETHERNET_CONF_GATEWAY_ADDR3);


#endif

	/* Add data to netif. Threaded lwIP (NO_SYS = 0): received frames are passed
	 * to the tcpip thread through tcpip_input. */
	if (NULL == netif_add(&gs_net_if, &x_ip_addr, &x_net_mask, &x_gateway, NULL,
			ethernetif_init, tcpip_input)) {
		LWIP_ASSERT("NULL == netif_add", 0);
	}

	/* Make it the default interface */
	netif_set_default(&gs_net_if);

	/* Setup callback function for netif status change */
	netif_set_status_callback(&gs_net_if, status_callback);

	/* Bring it up */
#if defined(DHCP_USED)
	/* DHCP mode. */
	if (ERR_OK != dhcp_start(&gs_net_if)) {
		LWIP_ASSERT("ERR_OK != dhcp_start", 0);
	}
	//printf("DHCP Started\r\n");
#else
	/* Static mode. */
	netif_set_up(&gs_net_if);
	//printf("\r\nStatic IP Address Assigned\r\n");
#endif
}

/**
 * \brief Initialize the lwIP TCP/IP stack with the network interface driver.
 */
void init_ethernet(void)
{
	/* Initialize lwIP in threaded mode: creates the tcpip thread, which also runs
	 * the lwIP timers (the NO_SYS timer table and ethernet_task() are gone).
	 * Called before vTaskStartScheduler(), so nothing runs concurrently yet. */
	tcpip_init(NULL, NULL);

	/* Set hw and IP parameters, initialize MAC too (the GMAC driver creates its
	 * own receive task when NO_SYS = 0). */
	ethernet_configure_interface();

	/* 1 ms tick for sys_get_ms()/sys_now() (TC0 channel 0). */
	sys_init_timing();
}

/**
 * \brief Callback function used to print the given IP address when the link
 * is up.
 *
 * \param netif The network interface instance.
 */
void status_callback(struct netif *netif)
{
	int8_t c_mess[20];
	if (netif_is_up(netif)) {
		strcpy((char*)c_mess, "IP=");
		strcat((char*)c_mess, inet_ntoa(*(struct in_addr *)&(netif->ip_addr)));
		//printf("\n\n\n\nNetwork up %s\r\n", (char const*)c_mess);
	} else {
		//printf("\nNetwork down\r\n");
	}
}

