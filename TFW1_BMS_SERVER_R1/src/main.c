/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Project Name  : TFW1_BMS_SERVER_R1
* Module Name	: Main.c
* Created By	: Anup Silvan Mascarenhas
* Created Date	: 20/12/2024.
* Module
* Description	: Initializing and testing of Peripherals
*
*
* Controller	: 	ATSAM4E16CA-AUR
*					1024 KB		Flash.
*					128 KB		RAM.
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: ----
* Changes		: NA.
*****************************************************************************/


#include "asf.h"
#include "sysclk.h"
#include "application.h"
#include "defination.h"

//extern bool gb_connect_cmd_rec ;
extern bool gb_start_timer ;
extern bool gb_timer ;
extern volatile uint16_t jsec ;

int main(void)
{
	wdt_disable(WDT);							/* Disable watch-dog timer   */
	 __enable_irq();
	sysclk_init();								/* Initializing system-clock */
	board_init();								/* Initializing board */
	Test_Func_Init();							/* Test Function Initialization */
	
//	if(check_switch_press() == true)  // Check Switch Press
//	{
//		Check_Boot_Command() ;     // Check Boot Command For jump to Bootloader
//
//	}

	while (1)
	{
		#if ETHERNET
		ethernet_task();
		#endif
		
		if(tc_flag)
		{
			if(ch_i > 0)
			{
				rx_str[ch_i] = '\0';
				Pheriheral_test_func();
				ch_i = 0;
				memset(rx_str,0,100);
			}
			tc_flag = 0;
		}
		if(rs485_flag)
		{
			if(rs_i > 0)
			{
				rx_rs485_buff[rs_i] = '\0';
				RS_485_C1_Test();
				rs_i = 0;
			}
			rs485_flag = 0;
		}
		
		if(rs485_2_flag)
		{
			if(rs_2_i > 0)
			{
				rx_rs485_2_buff[rs_2_i] = '\0';
				RS_485_C2_Task();
				rs_2_i = 0;
			}
			rs485_2_flag = 0;
		}
		Input_Test();
				
		if(gb_timer)
		{
			gb_start_timer = 0;
			gb_timer = false;
			#ifdef DEBUG_BOOT_APP
			Print_Message("\nBoot Command  Not Received......");
			#endif
		}
		if(jsec >=3)
		{
			Run_Bootlodar() ;      // Jump into Bootloader

		}
	}
}