/*****************************************************************************
* Module Name	: factory_cmd.h
* Description	: UART-only factory commands (TLS entropy seed provisioning).
*****************************************************************************/
#ifndef FACTORY_CMD_H_
#define FACTORY_CMD_H_

#include "compiler.h"

void Factory_Cmd_RxByte(U8 c);		/* from UART0_Handler: buffers one line */
void Factory_Cmd_Process(void);		/* from the client task: executes the line */

#endif /* FACTORY_CMD_H_ */
