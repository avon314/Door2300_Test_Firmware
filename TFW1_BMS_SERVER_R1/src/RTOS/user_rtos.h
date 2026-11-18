/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: user_rtos.h
* Description	: FreeRTOS task configuration of TFW1_BMS_SERVER_R1.
*
* Priorities (higher number = higher priority):
*	tcpip thread (lwIP)          tskIDLE_PRIORITY + 5   (lwipopts.h)
*	GMAC receive task            tskIDLE_PRIORITY + 4   (lwipopts.h)
*	Firmware_Server_Task (502)   tskIDLE_PRIORITY + 3
*	Test_Task (UART test menu)   tskIDLE_PRIORITY + 2
*	HTTPS_Client_Task            tskIDLE_PRIORITY + 1   (long TLS handshakes
*	                                                     must not starve the rest)
*****************************************************************************/
#ifndef USER_RTOS_H_
#define USER_RTOS_H_

#define TASK_TEST_STACK_WORDS			(1024u)		/* EEPROM tests use ~1 KB of locals */
#define TASK_TEST_PRIORITY				(tskIDLE_PRIORITY + 2)
#define TASK_FW_SERVER_STACK_WORDS		(512u)
#define TASK_FW_SERVER_PRIORITY			(tskIDLE_PRIORITY + 3)
#define TASK_HTTPS_CLIENT_PRIORITY		(tskIDLE_PRIORITY + 1)

void Create_Tasks(void);		/* Creates the application tasks (before vTaskStartScheduler). */
void Test_Task(void *pvParameters);	/* main.c */

#endif /* USER_RTOS_H_ */
