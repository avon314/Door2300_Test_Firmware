/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: CRC.h
* Created By	: Harshal Belan
* Copied By	: Harshit Agnihotri
* Copied Date	: 29/07/2024
*
* Module Description : Defines constants and macros and header file for CRC.c.
*
* REVISION HISTORY
* Version : 1.0
* Revision Date: 29/07/2024
* Changes : NA.
*****************************************************************************/
#ifndef CRC_H_
#define CRC_H_

#include <asf.h>

#define CRC_MAX_LEN		(1000)	// 1000 Bytes.
#define CRC_CALC	(1)                  //macro used for CRC calculation
#define CRC_CHECK	(0)                //macro used for CRC calculation

unsigned char CRC_Calculate_Check(unsigned char*Frame_Ptr, unsigned int Data_Limit, unsigned char Op_Type);

unsigned char gb_Temp_CRC[2];       //var used for CRC calculation
#endif /* CRC_H_ */