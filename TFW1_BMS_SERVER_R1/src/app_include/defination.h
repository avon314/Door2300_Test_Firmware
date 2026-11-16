/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: defination.h
* Created By	: Anup Silvan Mascarenhas.
* Created Date	: 20/12/2024.
* Module
* Description	: Header file for main.c
				  Defines macros for main.c.
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: ---
* Changes		: NO.
*****************************************************************************/


#ifndef DEFINATION_H_
#define DEFINATION_H_

/***********Including Header Files***********/
#include <asf.h>
#include <string.h>
#include "ethernet.h"
#include "httpd.h"
#include "user_timer.h"
#include "user_uart.h"
#include "digital_input.h"
#include "led_operation.h"
#include "relay_output.h"
#include "user_i2c.h"
#include "user_rtc.h"
#include "user_spi.h"
#include "flash_spi.h"
#include "ext_ram.h"
#include "ext_eeprom.h"
#include "ext_eeprom_1.h"
#include "rs485_driver.h"
#include "rs485_uart.h"
#include "rs485_driver_2.h"
#include "rs485_uart_2.h"
#include "bootloader.h"
#include "CRC.h"
#include "config_eth_ip_addr.h"


/************Defining MACROS**************/
#define UART_TEST 1
#define DEBUG_TEST_UART 1
#define DI_TEST 1
#define RELAY_TEST 1
#define RTC_INIT 1
#define FLASH 1
#define SRAM 1
#define EEPROM 1
#define RS485_C1 1
#define RS485_C2 1
#define ETHERNET 1

#endif /* DEFINATION_H_ */