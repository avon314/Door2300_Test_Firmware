/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: https_client.h
* Description	: Periodic HTTPS POST client task (HTTPS porting guide, section 7).
*****************************************************************************/
#ifndef HTTPS_CLIENT_H_
#define HTTPS_CLIENT_H_

/* Stack of the client task: 2048 words (8 KB), as on MM-01 (guide 6.8).
 * Trim it after measuring with uxTaskGetStackHighWaterMark (MM_MEM_STATS=1). */
#define HTTPS_CLIENT_STACK_WORDS	(2048u)

void HTTPS_Client_Task(void *pvParameters);

#endif /* HTTPS_CLIENT_H_ */
