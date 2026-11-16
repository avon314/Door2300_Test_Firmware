/*
 * config_eth_ip_addr.c
 *
 * Created: 26-07-2025 09:50:18 AM
 *  Author: admin
 */ 

#include <string.h>
#include "config_eth_ip_addr.h"
#include "defination.h"
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




extern Ip_Addr IpAddr;     // Declare the structure variable of Ip_Addr to store the ip address.
extern char Port_Num[6];   // Declare the variable to store the port number.
extern Sbnt_Msk SbntMsk;   // Declare the structure variable of Sbnt_Msk to store the subnet mask.
extern Dflt_Gtwy DfltGtwy; // Declare the structure variable of Dflt_Gtwy to store the default gateway.


void Set_eth_ip_address(void)
{
	extern struct ip_addr x_ip_addr, x_net_mask, x_gateway;
	
	U8 lclIPAddress[4] = {0};
	U8 lclSubNetMask[4] = {0};
	U8 lclDefGateway[4] = {0};
		
	char *endPtr;
	long int num = 0;
	U8 lclIdx = 0;
		
	/****************************************************/
	num = strtol(IpAddr.ip_addr_1, &endPtr, 10);
	lclIPAddress[lclIdx++] = (U8) num;
	num = strtol(IpAddr.ip_addr_2, &endPtr, 10);
	lclIPAddress[lclIdx++] = (U8) num;
	num = strtol(IpAddr.ip_addr_3, &endPtr, 10);
	lclIPAddress[lclIdx++] = (U8) num;
	num = strtol(IpAddr.ip_addr_4, &endPtr, 10);
	lclIPAddress[lclIdx++] = (U8) num;
		
	#if DEBUG_ETHERNET
	Print_Message("\nIP Address is : ");
	Print_Number(lclIPAddress[0]);
	Print_Message(":");
	Print_Number(lclIPAddress[1]);
	Print_Message(":");
	Print_Number(lclIPAddress[2]);
	Print_Message(":");
	Print_Number(lclIPAddress[3]);
	#endif
		
	/* Fixed IP mode. */
	/* Default IP addr */
	IP4_ADDR(&x_ip_addr, lclIPAddress[0], lclIPAddress[1],
	lclIPAddress[2], lclIPAddress[3]);
		
	/*******************************************************/
	lclIdx = 0;
	num = strtol(SbntMsk.sbnt_msk_1, &endPtr, 10);
	lclSubNetMask[lclIdx++] = (U8) num;
	num = strtol(SbntMsk.sbnt_msk_2, &endPtr, 10);
	lclSubNetMask[lclIdx++] = (U8) num;
	num = strtol(SbntMsk.sbnt_msk_3, &endPtr, 10);
	lclSubNetMask[lclIdx++] = (U8) num;
	num = strtol(SbntMsk.sbnt_msk_4, &endPtr, 10);
	lclSubNetMask[lclIdx++] = (U8) num;
		
	#if DEBUG_ETHERNET
	Print_Message("\nSubnet Mask is : ");
	Print_Number(lclSubNetMask[0]);
	Print_Message(":");
	Print_Number(lclSubNetMask[1]);
	Print_Message(":");
	Print_Number(lclSubNetMask[2]);
	Print_Message(":");
	Print_Number(lclSubNetMask[3]);
	#endif
		
	/* Default subnet mask. */
	IP4_ADDR(&x_net_mask, lclSubNetMask[0], lclSubNetMask[1],
	lclSubNetMask[2], lclSubNetMask[3]);
		
	/*******************************************************/
	lclIdx = 0;
	num = strtol(DfltGtwy.dflt_gtwy_1, &endPtr, 10);
	lclDefGateway[lclIdx++] = (U8) num;
	num = strtol(DfltGtwy.dflt_gtwy_2, &endPtr, 10);
	lclDefGateway[lclIdx++] = (U8) num;
	num = strtol(DfltGtwy.dflt_gtwy_3, &endPtr, 10);
	lclDefGateway[lclIdx++] = (U8) num;
	num = strtol(DfltGtwy.dflt_gtwy_4, &endPtr, 10);
	lclDefGateway[lclIdx++] = (U8) num;
		
	#if DEBUG_ETHERNET
	Print_Message("\nDefault gateway is : ");
	Print_Number(lclDefGateway[0]);
	Print_Message(":");
	Print_Number(lclDefGateway[1]);
	Print_Message(":");
	Print_Number(lclDefGateway[2]);
	Print_Message(":");
	Print_Number(lclDefGateway[3]);
	#endif
		
	/* Default gateway addr. */
	IP4_ADDR(&x_gateway, lclDefGateway[0], lclDefGateway[1],
	lclDefGateway[2], lclDefGateway[3]);
}


void Read_IP_Adress_From_EEP1(void)
{
	U8 read_data[255] = {0};
	U8 len = 255;
	memset(read_data, 0, len);
	// Read the data including CRC from flash memory
	EEPROM_Page_Read(UT_EEP_START_PAGE, 0, read_data, len);
	
	if (len <= UTLTY_FRM_MX_SZ)
	{
		// Check CRC of the read data
		if (CRC_Calculate_Check(read_data, (len - 2), CRC_CHECK) == 1)
		{
			// If CRC verified, write the respective data in RAM
			U8 received_comn_data_idx = 0;

			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)                    // Parse and write the ip address.
			IpAddr.ip_addr_1[ip_addr_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			IpAddr.ip_addr_2[ip_addr_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			IpAddr.ip_addr_3[ip_addr_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			IpAddr.ip_addr_4[ip_addr_idx] = read_data[received_comn_data_idx++];
			IpAddr.ip_addr_1[3] = '\0';
			IpAddr.ip_addr_2[3] = '\0';
			IpAddr.ip_addr_3[3] = '\0';
			IpAddr.ip_addr_4[3] = '\0';

			//      for(U8 port_num_idx = 0; port_num_idx < 5; port_num_idx++)                 // Parse and write the port number.
			//      Port_Num[port_num_idx] = read_data[received_comn_data_idx++];

			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)                 // Parse and write the subnet mask.
			SbntMsk.sbnt_msk_1[sbnt_msk_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			SbntMsk.sbnt_msk_2[sbnt_msk_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			SbntMsk.sbnt_msk_3[sbnt_msk_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			SbntMsk.sbnt_msk_4[sbnt_msk_idx] = read_data[received_comn_data_idx++];
			SbntMsk.sbnt_msk_1[3] = '\0';
			SbntMsk.sbnt_msk_2[3] = '\0';
			SbntMsk.sbnt_msk_3[3] = '\0';
			SbntMsk.sbnt_msk_4[3] = '\0';

			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)              // Parse and write the default gateway.
			DfltGtwy.dflt_gtwy_1[dflt_gtwy_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			DfltGtwy.dflt_gtwy_2[dflt_gtwy_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			DfltGtwy.dflt_gtwy_3[dflt_gtwy_idx] = read_data[received_comn_data_idx++];
			received_comn_data_idx++;
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			DfltGtwy.dflt_gtwy_4[dflt_gtwy_idx] = read_data[received_comn_data_idx++];
			DfltGtwy.dflt_gtwy_1[3] = '\0';
			DfltGtwy.dflt_gtwy_2[3] = '\0';
			DfltGtwy.dflt_gtwy_3[3] = '\0';
			DfltGtwy.dflt_gtwy_4[3] = '\0';
		}
		else
		{
			// If CRC check fails, write default data to RAM
			memset(read_data, 0, len);

			//#if DEBUG_UTILITY
			Print_Message("\nSetting Default Configuration");
			//#endif

			strcpy(IpAddr.ip_addr_1, "192");                                           // Write the default values for ip address.
			strcpy(IpAddr.ip_addr_2, "168");
			strcpy(IpAddr.ip_addr_3, "001");
			strcpy(IpAddr.ip_addr_4, "045");
			IpAddr.ip_addr_1[3] = '\0';
			IpAddr.ip_addr_2[3] = '\0';
			IpAddr.ip_addr_3[3] = '\0';
			IpAddr.ip_addr_4[3] = '\0';

			strcpy(Port_Num, "00502");                                                 // Write the default values for port number.
			Port_Num[5] = '\0';

			strcpy(SbntMsk.sbnt_msk_1, "255");                                         // Write the default values for subnet mask.
			strcpy(SbntMsk.sbnt_msk_2, "255");
			strcpy(SbntMsk.sbnt_msk_3, "255");
			strcpy(SbntMsk.sbnt_msk_4, "000");
			SbntMsk.sbnt_msk_1[3] = '\0';
			SbntMsk.sbnt_msk_2[3] = '\0';
			SbntMsk.sbnt_msk_3[3] = '\0';
			SbntMsk.sbnt_msk_4[3] = '\0';

			strcpy(DfltGtwy.dflt_gtwy_1, "192");                                       // Write the default values for default gateway.
			strcpy(DfltGtwy.dflt_gtwy_2, "168");
			strcpy(DfltGtwy.dflt_gtwy_3, "001");
			strcpy(DfltGtwy.dflt_gtwy_4, "001");
			DfltGtwy.dflt_gtwy_1[3] = '\0';
			DfltGtwy.dflt_gtwy_2[3] = '\0';
			DfltGtwy.dflt_gtwy_3[3] = '\0';
			DfltGtwy.dflt_gtwy_4[3] = '\0';

			U8 read_comn_data_idx = 0;

			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)                     // Read and pass the ip address.
			read_data[read_comn_data_idx++] = IpAddr.ip_addr_1[ip_addr_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			read_data[read_comn_data_idx++] = IpAddr.ip_addr_2[ip_addr_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			read_data[read_comn_data_idx++] = IpAddr.ip_addr_3[ip_addr_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 ip_addr_idx = 0; ip_addr_idx < 3; ip_addr_idx++)
			read_data[read_comn_data_idx++] = IpAddr.ip_addr_4[ip_addr_idx];

			//      for(U8 port_num_idx = 0; port_num_idx < 5; port_num_idx++)                  // Read and pass the port number.
			//      read_data[read_comn_data_idx++] = Port_Num[port_num_idx];

			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)                  // Read and pass the subnet mask.
			read_data[read_comn_data_idx++] = SbntMsk.sbnt_msk_1[sbnt_msk_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			read_data[read_comn_data_idx++] = SbntMsk.sbnt_msk_2[sbnt_msk_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			read_data[read_comn_data_idx++] = SbntMsk.sbnt_msk_3[sbnt_msk_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 sbnt_msk_idx = 0; sbnt_msk_idx < 3; sbnt_msk_idx++)
			read_data[read_comn_data_idx++] = SbntMsk.sbnt_msk_4[sbnt_msk_idx];

			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)               // Read and pass the default gateway.
			read_data[read_comn_data_idx++] = DfltGtwy.dflt_gtwy_1[dflt_gtwy_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			read_data[read_comn_data_idx++] = DfltGtwy.dflt_gtwy_2[dflt_gtwy_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			read_data[read_comn_data_idx++] = DfltGtwy.dflt_gtwy_3[dflt_gtwy_idx];
			read_data[read_comn_data_idx++] = '.';
			for(U8 dflt_gtwy_idx = 0; dflt_gtwy_idx < 3; dflt_gtwy_idx++)
			read_data[read_comn_data_idx++] = DfltGtwy.dflt_gtwy_4[dflt_gtwy_idx];

			CRC_Calculate_Check(read_data, (len - 2), CRC_CALC);
			read_data[(len - 2)] = gb_Temp_CRC[0];	// Higer Byte.
			read_data[(len - 1)] = gb_Temp_CRC[1];	// Lower Byte.
			EEPROM_Page_Write(UT_EEP_START_PAGE, 0, read_data, len);  // write to flash
		}
	}
	else
	{
		__NOP(); // Do nothing if length exceeds max frame size
	}
}