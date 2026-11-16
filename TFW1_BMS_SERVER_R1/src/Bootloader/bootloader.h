/*
 * bootloader.h
 *
 * Created: 14-07-2025 10:52:34 AM
 *  Author: admin
 */


#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "asf.h"
#include "user_uart.h"

extern U8 gb_ack_buff[30];
extern bool gb_download_frame;

#define BOOT_START_ADDRESS       (0x00400000)		// Define the starting address of the application in flash memory
#define STATUS_FLAG_ADDRESS      (3537408)        // EEPROM address for storing Status of boot loader .  //page no. 6909
#define DEBUG_BOOT_APP		   (1)
#define MAX_BIN_BYTES		   (600)

#define NEW_FIRM_VALUE		   (0x01)
#define UPDT_FIRM_VALUE		   (0x00)

enum {
	BTLD_SOF       = 0x23,
	BTLD_EOF       = 0X24,
	BTLD_DEST_ADD  = 0x02,
	BTLD_SRC_ADD   = 0x01,
	ACK_FRM_LEN    = 0x000A,
	FRM_LEN		= 0x000A,
	FUNC_ID		= 0X84

}BTLD_FRM;

enum{
	CRC_OK    = 0x00,
	CRC_NOK   = 0x01,
	LEN_NOK   = 0x02,
	DATA_NOK  = 0x03
}BTLD_ACK_ERR;

void Jump_To_Bootloader(void) ;
void Run_Bootlodar(void) ;
void Check_Boot_Command(void) ;
void Download_New_Firmware(U8 *bin_buffer, int Byte_Count) ;
void deinit_peripherals(void) ;
void btld_ack(U8 err, bool fuc_id) ;




#endif /* BOOTLOADER_H_ */