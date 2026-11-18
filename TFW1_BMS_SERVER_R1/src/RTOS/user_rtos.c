/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: user_rtos.c
* Description	: FreeRTOS task creation and the hook functions required by
*				  FreeRTOSConfig.h (malloc failed, stack overflow).
*				  Based on MM-01 "RTOS Files/user_rtos.c".
*
* Controller	: 	ATSAM4E16CA-AUR
*					1024 KB		Flash.
*					128 KB		RAM.
*****************************************************************************/
#include "asf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "user_uart.h"
#include "httpd.h"
#include "https_client.h"
#include "user_rtos.h"

void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName);

/*****************************************************************************
* Function name	: void vApplicationMallocFailedHook(void)
* Description	: Called when pvPortMalloc() fails (configUSE_MALLOC_FAILED_HOOK):
*				  the FreeRTOS heap (configTOTAL_HEAP_SIZE) is too small.
*****************************************************************************/
void vApplicationMallocFailedHook(void)
{
	Print_Message("\nFreeRTOS heap exhausted (configTOTAL_HEAP_SIZE).");
	taskDISABLE_INTERRUPTS();
	for (;;)
	{
	}
}

/*****************************************************************************
* Function name	: void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
* Description	: Called on a detected task stack overflow
*				  (configCHECK_FOR_STACK_OVERFLOW = 2).
*****************************************************************************/
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void)pxTask;
	taskDISABLE_INTERRUPTS();
	Print_Message("\nStack overflow detected in task: ");
	Print_Message((pcTaskName != NULL) ? (const char *)pcTaskName : "?");
	for (;;)
	{
	}
}

/*****************************************************************************
* Function name	: void Create_Tasks(void)
* Description	: Creates the application tasks. The lwIP tcpip thread and the
*				  GMAC receive task are created by init_ethernet().
*****************************************************************************/
void Create_Tasks(void)
{
	if (xTaskCreate(Test_Task, (const signed char *const)"Test",
					TASK_TEST_STACK_WORDS, NULL, TASK_TEST_PRIORITY, NULL) != pdPASS)
	{
		Print_Message("\nFailed to create Test task.");
	}

	if (xTaskCreate(Firmware_Server_Task, (const signed char *const)"FwServer",
					TASK_FW_SERVER_STACK_WORDS, NULL, TASK_FW_SERVER_PRIORITY, NULL) != pdPASS)
	{
		Print_Message("\nFailed to create port-502 server task.");
	}

	if (xTaskCreate(HTTPS_Client_Task, (const signed char *const)"HTTPS",
					HTTPS_CLIENT_STACK_WORDS, NULL, TASK_HTTPS_CLIENT_PRIORITY, NULL) != pdPASS)
	{
		Print_Message("\nFailed to create HTTPS client task.");
	}
}
