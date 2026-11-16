/*
 * config_eth_ip_addr.h
 *
 * Created: 26-07-2025 09:49:43 AM
 *  Author: admin
 */ 


#ifndef CONFIG_ETH_IP_ADDR_H_
#define CONFIG_ETH_IP_ADDR_H_

#include "asf.h"
#define DEBUG_ETHERNET 1

/***** Macros for Configuration Data *****/
#define UT_EEP_START_PAGE		(22)		// Page 22 is used to store configuration data (Utility)
#define UT_EEP_PAGE_OFFSET		(0)
#define UT_EEP_START_LOC			(11264)	// Start address (Byte location for page 22).
#define UTLTY_FRM_MX_SZ 			(0x0258)
/***** End of macros for Configuration Data *****/

void Read_IP_Adress_From_EEP1(void);
void Set_eth_ip_address(void) ;


#endif /* CONFIG_ETH_IP_ADDR_H_ */