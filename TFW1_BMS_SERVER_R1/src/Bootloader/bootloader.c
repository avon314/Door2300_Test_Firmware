/*
 * bootloader.c
 *
 * Created: 14-07-2025 10:52:15 AM
 *  Author: admin
 */
#include "bootloader.h"
#include "defination.h"
#include "ext_eeprom_1.h"


bool gb_download_frame    = false ;
bool switch_press         =false ;
U8 boot_flag[8]           = {0};
U8 gb_ack_buff[30]        = {0}; 
U8 Frame_Len              = 0;

extern bool gb_boot_jump_start_timer ;
extern bool gb_start_timer ;
extern bool gb_timer ;
extern bool switch_press ;
extern volatile uint16_t jsec ;


/*****************************************************************************
* Function name	: void Jump_To_Bootloader( void)
* Returns		: Nothing.
* Arguments    	: Nothing.
* Created By	: Suryaprakash Parmeshwar Shendge
* Created Date	: 14/07/2025.
*
*
* Description	:  Function to jump to bootloader.
*
*               :
* Notes			: NA
* Global Variables Affected : None .
*****************************************************************************/
void Jump_To_Bootloader(void)
{
	uint32_t MSP = *(uint32_t *)(BOOT_START_ADDRESS) ;
	uint32_t reset_vector = *(uint32_t *)(BOOT_START_ADDRESS + 4);

	deinit_peripherals();

	// Clear pending system exceptions (PendSV, SysTick, etc.)
	SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
	SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;

	SCB->VTOR = BOOT_START_ADDRESS;          // Set vector table

	__set_MSP(MSP);                  // Set Main Stack Pointer
	Print_Message("\nMSP Value : ");
	Print_Number(MSP);
	__set_CONTROL(0);                // Use MSP, privileged mode
	__ISB();                         // Flush instruction pipeline

	void (*app_reset_handler)(void) = (void (*)(void))reset_vector;
	__enable_irq();			// enable all interrupts before jumping to application
	app_reset_handler();


}

/*****************************************************************************
* Function name	: void Run_Bootlodar(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created By	: Suryaprakash Parmeshwar Shendge
* Created Date	: 14/07/2025..
*
*
* Description	:  Function to jump from application to bootloade
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Run_Bootlodar(void)
{
	#if DEBUG_BOOT_APP
	Print_Message("\n\nJumping To Bootloader............. ");
	#endif
	boot_flag[0] = NEW_FIRM_VALUE;    // Update Boot flag
	//Flash_Byte_Write(0, boot_flag, 8); // Write Boot flag into external Flash
	EEPROM_Byte_Write(STATUS_FLAG_ADDRESS,&boot_flag,8) ;

	tc_stop(TC0, 0);          // Stop Timer Counter 0 (assuming TC0 was used by bootloader)
	tc_stop(TC0, 1);         // Stop Timer Counter 1 (assuming TC0 was used by bootloader)

	__disable_irq();        // Disable all interrupts before jumping to application

	Jump_To_Bootloader() ;

}

/*****************************************************************************
* Function name	: void Check_Boot_Command(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created By	: Suryaprakash Parmeshwar Shendge
* Created Date	: 14/07/2025..
*
*
* Description	:  Function checking boot command is receive
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Check_Boot_Command(void)
{
	switch_press = true ;
	OFF_RED_LED;
	#ifdef DEBUG_BOOT_APP
	Print_Message("\n\nWating for Boot Command......");
	gb_start_timer = true ;
	#endif
	while (1)
	{
		if(gb_timer)
		{
			#ifdef DEBUG_BOOT_APP
			Print_Message("\nBoot Command  Not Received......");
			#endif
			break;
		}

		ethernet_task();               // Handle Ethernet communication

		if(jsec >=3)
		{
			Run_Bootlodar() ;      // Jump into Bootloader

		}
	}

}


/*****************************************************************************
* Function name	: void Check_Boot_Command(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created By	: Suryaprakash Parmeshwar Shendge
* Created Date	: 07/11/2025..
*
*
* Description	:  Function checking boot command is receive
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Download_New_Firmware(U8 *bin_buffer, int Byte_Count)
{
	if((bin_buffer[5] == 0x84))
	{
		
		Print_Message("\r\nFirmware Update Command Received ") ;
		
		//U8 updt_firmware[ACK_FRM_LEN] = {0};

		gb_ack_buff[0] = BTLD_SOF;
		gb_ack_buff[1] = ((ACK_FRM_LEN >> 8)& 0xFF);
		gb_ack_buff[2] = (ACK_FRM_LEN & 0xFF);
		gb_ack_buff[3] = 0x01;
		gb_ack_buff[4] = 0x02;
		gb_ack_buff[5] = 0x00;
		gb_ack_buff[6] = 0x00;
		gb_ack_buff[7] = BTLD_EOF;

		if(CRC_Calculate_Check(gb_ack_buff,(ACK_FRM_LEN - 2),CRC_CALC))
		{
			gb_ack_buff[ACK_FRM_LEN - 3] = gb_Temp_CRC[0];
			gb_ack_buff[ACK_FRM_LEN - 2] = gb_Temp_CRC[1];
			gb_ack_buff[ACK_FRM_LEN - 1] = BTLD_EOF;
			gb_boot_jump_start_timer = true ;                     // Set timer start flag to jump to bootloader
		}
		else
		{
			gb_download_frame = false;

			#if DEBUG_BOOT_APP
			Print_Message("Frame NOK");
			#endif
		}

		#if DEBUG_BOOT_APP
//		Send_Frame_On_UART(bin_buffer,Byte_Count);
//		Send_Frame_On_UART(gb_ack_buff,ACK_FRM_LEN);
		#endif

//		if(memcmp(gb_ack_buff,bin_buffer,Byte_Count) == 0) 
//		{
//			//btld_ack(CRC_OK,true);
//		}
//		else
//		{
//			//btld_ack(CRC_NOK,false);
//			gb_download_frame = false;
//
//			#if DEBUG_BOOT_APP
//			Print_Message("Frame NOK");
//			#endif
//		}
	
	}
	else if ((bin_buffer[5] == 0x82)/* && (switch_press == true)*/)
	{
		Print_Message("\r\nConnect Command Received ") ;
/*		gb_download_frame = true;
//		
//		U8 updt_firmware[ACK_FRM_LEN] = {0};
//
//		updt_firmware[0] = BTLD_SOF;
//		updt_firmware[1] = ((0x0021 >> 8)& 0xFF);
//		updt_firmware[2] = (0x0021 & 0xFF);
//		updt_firmware[3] = 0x01;
//		updt_firmware[4] = 0x02;
//		updt_firmware[5] = FUNC_ID;
//		updt_firmware[6] = BTLD_EOF;
//
//		CRC_Calculate_Check(updt_firmware,(ACK_FRM_LEN - 2),CRC_CALC);
//		updt_firmware[6] = gb_Temp_CRC[0];
//		updt_firmware[7] = gb_Temp_CRC[1];
//		updt_firmware[8] = BTLD_EOF;
//
//		#if DEBUG_BOOT_APP
//		Send_Frame_On_UART(bin_buffer,Byte_Count);
//		Send_Frame_On_UART(updt_firmware,ACK_FRM_LEN);
//		#endif
//	
//          U8 len     = 0x0013 ;
//		Frame_Len  = 0x0013 ;
//			
//		gb_ack_buff[0] = BTLD_SOF;
//		gb_ack_buff[1] = (len >> 8) & 0xFF;
//		gb_ack_buff[2] = len & 0xFF;
//		gb_ack_buff[3] = 0x02;
//		gb_ack_buff[4] = 0x01;
//
//		gb_ack_buff[5] = 0x82;
//		
//		gb_ack_buff[6] = 0x00;
//		gb_ack_buff[7] = 0x00;
//		gb_ack_buff[8] = 0x00;
//		gb_ack_buff[9] = 0x00;
//		gb_ack_buff[10] = 0x00;
//		gb_ack_buff[11] = 0x00;
//		gb_ack_buff[12] = 0x00;
//		gb_ack_buff[13] = 0x00;
//		gb_ack_buff[14] = 0x00;
//		
//		
//		gb_ack_buff[15] = 0x00;
//				
//
//		#if DEBUG_NEW_APP
//		Print_Message("\nokokokokokok ______>>>>>");
//		Send_Frame_On_UART(gb_ack_buff,(len - 2));
//		#endif
//
//		CRC_Calculate_Check(gb_ack_buff, (len - 2) , CRC_CALC);
//
//		#if DEBUG_NEW_APP
//		Print_Message("\n??????????? : ");
//		Print_ASCII_HEX(gb_Temp_CRC[0]);
//		Print_Message(" : ");
//		Print_ASCII_HEX(gb_Temp_CRC[1]);
//		#endif
//
//		gb_ack_buff[16]     = 0x30;
//		gb_ack_buff[17]     = 0xA6;
//		gb_ack_buff[18]     = BTLD_EOF;
//
//		#if DEBUG_NEW_APP
//		Print_Message("\nAck ______>>>>>");
//		Send_Frame_On_UART(gb_ack_buff,len);
//		#endif
*/
          char conn_dvc_str[20];             // Char array to store connect with device string.
          char conn_str[9] = {"Connected"};
          U16 tx_frame_idx = 0;                                       // Index to track the received and transmitted frame.
		
		for(U8 data_idx = 0; data_idx < 20; data_idx++)     // Parse the received string for connect with device.
		{
			conn_dvc_str[data_idx] = bin_buffer[data_idx + 6];
		}
		
		Frame_Len  = 0x0013 ;
		gb_ack_buff[tx_frame_idx++] = 0x23;            // Start building the transmit frame (SOF).
		gb_ack_buff[tx_frame_idx++] = (Frame_Len >> 8) & 0xFF;
		gb_ack_buff[tx_frame_idx++] = Frame_Len & 0xFF;
		
		gb_ack_buff[tx_frame_idx++] = 0x01;    // Set destination and source addresses.
		gb_ack_buff[tx_frame_idx++] = 0x02;
		gb_ack_buff[tx_frame_idx++] = 0x82;      // Set function ID.
		for(U8 data_idx = 0; data_idx < 9; data_idx++)   // Fill the transmit frame with the response string.
		{
			gb_ack_buff[tx_frame_idx++] = conn_str[data_idx];
		}
		gb_ack_buff[tx_frame_idx++] = 0x00;       // Set error byte and EOF.
		gb_ack_buff[tx_frame_idx++] = 0x24;
		
		if(CRC_Calculate_Check(gb_ack_buff, Frame_Len - 2, CRC_CALC))         // Calculate and set CRC and add EOF at the end.
		{
			gb_ack_buff[Frame_Len - 3] = gb_Temp_CRC[0];
			gb_ack_buff[Frame_Len - 2] = gb_Temp_CRC[1];
			gb_ack_buff[Frame_Len - 1] = 0x24;
			
			OFF_RED_LED;
			#ifdef DEBUG_BOOT_APP
			Print_Message("\n\nWating for Boot Command......");
			#endif
			gb_start_timer = true ;
		}
		
		#if DEBUG_ALL
		Print_Message("\nFrame sent on tcp server is = ");
		Send_Frame_On_UART(gb_ack_buff, Frame_Len);
		#endif
		
	}
	else if(bin_buffer[5] == 0x85)
	{
		Print_Message("\r\nExit Command Received ") ;
		char conn_dvc_str[16];             // Char array to store connect with device string.
		char conn_str[14] = {"Config_is_Exit"};
		U16 tx_frame_idx = 0;                                       // Index to track the received and transmitted frame.
		
		for(U8 data_idx = 0; data_idx < 16; data_idx++)     // Parse the received string for connect with device.
		{
			conn_dvc_str[data_idx] = bin_buffer[data_idx + 6];
		}
		
		Frame_Len  = 0x0018 ;
		gb_ack_buff[tx_frame_idx++] = 0x23;            // Start building the transmit frame (SOF).
		gb_ack_buff[tx_frame_idx++] = (Frame_Len >> 8) & 0xFF;
		gb_ack_buff[tx_frame_idx++] = Frame_Len & 0xFF;
		
		gb_ack_buff[tx_frame_idx++] = 0x01;    // Set destination and source addresses.
		gb_ack_buff[tx_frame_idx++] = 0x02;
		gb_ack_buff[tx_frame_idx++] = 0x85;      // Set function ID.
		for(U8 data_idx = 0; data_idx < 14; data_idx++)   // Fill the transmit frame with the response string.
		{
			gb_ack_buff[tx_frame_idx++] = conn_str[data_idx];
		}
		gb_ack_buff[tx_frame_idx++] = 0x00;       // Set error byte and EOF.
		gb_ack_buff[tx_frame_idx++] = 0x24;
		
		if(CRC_Calculate_Check(gb_ack_buff, Frame_Len - 2, CRC_CALC))         // Calculate and set CRC and add EOF at the end.
		{
			gb_ack_buff[Frame_Len - 3] = gb_Temp_CRC[0];
			gb_ack_buff[Frame_Len - 2] = gb_Temp_CRC[1];
			gb_ack_buff[Frame_Len - 1] = 0x24;
		}
		
		#if DEBUG_ALL
		Print_Message("\nFrame sent on tcp server is = ");
		Send_Frame_On_UART(gb_ack_buff, Frame_Len);
		#endif
	}
}

/*****************************************************************************
* Function name	: void deinit_peripherals(void)
* Returns			: NA
* Arguments    	: NA
* Created by		: Anup Silvan Mascarenhas
* Date created		: 11/04/2025.
*
* Description		: This function disables IRQ when booted to new firmware
*				:
* Notes			: NA
* Global Variables Affected : NA
*****************************************************************************/
void deinit_peripherals(void) {
	// Disable SysTick
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	// Disable all NVIC interrupts
	for (int i = 0; i < 8; i++) {
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	// Optional: reset peripherals if needed
	// PM->PM_CTRLA.bit.SWRST = 1; // Example for SAMD, not SAM4E
}

/*****************************************************************************
* Function name	: void btld_ack(U8 err, bool fuc_id)
* Returns			: NA
* Arguments    	: U8 err		---> Error code
*				: bool fuc_id	---> runs in two functionalities
* Created by		: Anup Silvan Mascarenhas
* Date created		: 11/04/2025.
*
* Description		: This Function is written to send back ack to tcp-ip
*				:
*				:
* Notes			: NA
* Global Variables Affected : gb_fRead_Array	---> Buffer received from Extrenal Flash
*****************************************************************************/
void btld_ack(U8 err, bool fuc_id)
{
	U8 len     = fuc_id ? FRM_LEN : ACK_FRM_LEN;
	U8 crc_idx = fuc_id ? 7 : 6;
	U8 eof_idx = crc_idx + 2;
	
	Frame_Len  =  len ;

	gb_ack_buff[0] = BTLD_SOF;
	gb_ack_buff[1] = (len >> 8) & 0xFF;
	gb_ack_buff[2] = len & 0xFF;
	gb_ack_buff[3] = BTLD_DEST_ADD;
	gb_ack_buff[4] = BTLD_SRC_ADD;
	gb_ack_buff[5] = err;

	if (fuc_id)
	{
		gb_ack_buff[6] = FUNC_ID;
		gb_ack_buff[7] = BTLD_EOF;
	}
	else
	gb_ack_buff[6] = BTLD_EOF;

	#if DEBUG_NEW_APP
	Print_Message("\nokokokokokok ______>>>>>");
	Send_Frame_On_UART(gb_ack_buff,(len - 2));
	#endif

	CRC_Calculate_Check(gb_ack_buff, (len - 2) , CRC_CALC);

	#if DEBUG_NEW_APP
	Print_Message("\n??????????? : ");
	Print_ASCII_HEX(gb_Temp_CRC[0]);
	Print_Message(" : ");
	Print_ASCII_HEX(gb_Temp_CRC[1]);
	#endif

	gb_ack_buff[crc_idx]     = gb_Temp_CRC[1];
	gb_ack_buff[crc_idx + 1] = gb_Temp_CRC[0];
	gb_ack_buff[eof_idx]     = BTLD_EOF;

	#if DEBUG_NEW_APP
	Print_Message("\nAck ______>>>>>");
	Send_Frame_On_UART(gb_ack_buff,len);
	#endif
	
	gb_boot_jump_start_timer = true ;                     // Set timer start flag to jump to bootloader

}