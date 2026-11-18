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
*
* Revision Date	: 16/11/2026
* Changes		: HTTPS client (HTTPS porting guide, MM-01). The firmware now runs
*				  on FreeRTOS 7.3 with threaded lwIP (netconn): the former
*				  superloop is Test_Task(), the port-502 server is
*				  Firmware_Server_Task() and the HTTPS POST is HTTPS_Client_Task().
*****************************************************************************/


#include "asf.h"
#include "sysclk.h"
#include "application.h"
#include "defination.h"
#include "FreeRTOS.h"
#include "task.h"
#include "user_rtos.h"

//extern bool gb_connect_cmd_rec ;
extern bool gb_start_timer ;
extern bool gb_timer ;
extern volatile uint16_t jsec ;

/*****************************************************************************
* Function name	: int main(void)
* Description	: Initialises the clocks, board, peripherals and lwIP, creates
*				  the tasks and starts the FreeRTOS scheduler.
*****************************************************************************/
int main(void)
{
	wdt_disable(WDT);							/* Disable watch-dog timer   */
	 __enable_irq();
	sysclk_init();								/* Initializing system-clock */
	NVIC_SetPriorityGrouping(0);				/* All priority bits are preemption bits (FreeRTOS) */
	board_init();								/* Initializing board */
	Test_Func_Init();							/* Test Function Initialization (also lwIP / tcpip thread) */

	Create_Tasks();								/* Test, port-502 server and HTTPS client tasks */
	vTaskStartScheduler();						/* Does not return */

	/* Only reached if the FreeRTOS heap is too small for the idle task. */
	Print_Message("\nvTaskStartScheduler failed: FreeRTOS heap too small.");
	for (;;)
	{
	}
}

/*****************************************************************************
* Function name	: void Test_Task(void *pvParameters)
* Description	: The former main() superloop: UART test menu, RS485 channels,
*				  inputs, boot timer. ethernet_task() is gone (the tcpip thread
*				  and the GMAC task handle the network now).
*****************************************************************************/
void Test_Task(void *pvParameters)
{
	UNUSED(pvParameters);

	/* (Moved from main(): Check_Boot_Command() waits for the port-502 boot
	 *  command, so with the RTOS it must run inside a task.) */
//	if(check_switch_press() == true)  // Check Switch Press
//	{
//		Check_Boot_Command() ;     // Check Boot Command For jump to Bootloader
//
//	}

	while (1)
	{
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

		vTaskDelay(1);		/* let the lower-priority HTTPS client task run */
	}
}
