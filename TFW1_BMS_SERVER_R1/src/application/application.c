/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: application.c
* Created By	: Anup Silvan Mascarenhas
* Created Date	: 20/12/2024.
* Module
* Description	: This module implements the initialization and 
*				  testing of various peripherals like UART, 
*				  digital inputs, relays, RS485, RTC,
*				  flash memory, SRAM, and EEPROM.
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
#include "defination.h"
#include "application.h"

volatile uint8_t tc_flag = 0;
volatile uint8_t rx_flag = 0;
volatile char rx_str[100];
volatile uint8_t ch_i = 0;

volatile uint8_t input_set_flag = 0;
volatile uint8_t input_flag = 0;
volatile bool error_reported = false;

volatile uint8_t rtc_set_flag = 0;
volatile uint8_t day, month, year, hour, minute, second;

volatile uint8_t write_data[] = "Hello World";
volatile uint8_t flash_flag = 0;

volatile uint8_t sram_flag = 0;

volatile uint8_t rs_i = 0;
volatile char rx_rs485_buff[100];
volatile uint8_t rs485_flag = 0;
volatile uint8_t rx_485_flag = 0;

volatile uint8_t rs_2_i = 0;
volatile char rx_rs485_2_buff[100];
volatile uint8_t rs485_2_flag = 0;
volatile uint8_t rx_485_2_flag = 0;

extern bool gb_start_1_min_timer ;
extern bool gb_1_min_elapsed ;


/*****************************************************************************
* Function name	: void Test_Func_Init(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: Initialization of UART,digital inputs, relays, RS485, RTC,
*				  flash memory, SRAM, and EEPROM.
*          
* Global Variables Affected : NA.
*****************************************************************************/
void Test_Func_Init(void){
	configure_console();
	UART_Debug_Init();
	Timer_Init();
	Digital_Input_Driver_Init();
	Relay_Output_Driver_Init();
	Configure_LED_PortPins();
	configure_twi();
	RTC_Interrupt_Pin_Configure();
	configure_spi_master();
	Flash_Initialization();
	Configure_CS_Pin_For_extRAM();
	eeprom_pin_config();
	RS485_UART_Init();
	RS485_Driver_Init();
	//RS485_UART2_Init();
	//RS485_Driver_2_Init();
	init_ethernet();
	httpd_init();
	Print_Message("Welcome\n");
	Get_RTC_Data_At_Every_Second();
	EXT_EEPROM_Init() ;
//	U8 x = 0xFF ;
//	EEPROM_Byte_Write(STATUS_FLAG_ADDRESS,&(x),1);
	Read_IP_Adress_From_EEP1();
	OFF_RED_LED;
}

/*****************************************************************************
* Function name	: void Pheriheral_test_func(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
*
* Description	: Function call for testing all peripherals, 
* 				  such as UART, I/O pins, RS485, RTC, Flash, EEPROM, SRAM.
* 					
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void Pheriheral_test_func(void){

	#if UART_TEST
	UART_Task();
	#endif

	#if DI_TEST
	Digial_input_task();
	#endif

	#if RELAY_TEST
	relay_output();
	#endif
	
	#if RS485_C1
	RS_485_C1_Test();
	#endif
	
	#if RS485_C2
	RS_485_C2_Task();
	#endif
	
	#if RTC_INIT
	Real_Time_Clock();
	#endif
	
	#if FLASH
	Flash_Test();
	#endif
	
	#if SRAM
	SRAM_Test();
	#endif
	
	#if EEPROM
	EEPROM_Test();
	EEPROM_1_Test();
	#endif
}

/*****************************************************************************
* Function name	: void UART_Task(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: UART Test Communication Process Function
* 					
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void UART_Task(void){
	if(strcmp(rx_str, "UART_TX")==0){
		#if DEBUG_TEST_UART
		Print_Message("\nUART TX SUCCESFULL");
		#endif
	}
	if(strcmp(rx_str, "UART_RX")==0){
		rx_flag = 1;
	}
	if((rx_flag == 1) && (strcmp(rx_str, "UART_RX_FRAME")==0)){
		#if DEBUG_TEST_UART
		Print_Message("\nUART RX SUCCESFULL");
		#endif
		rx_flag = 0;
	}
}

/*****************************************************************************
* Function name	: void Digial_input_task(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: Digital Input Test Process Function
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void Digial_input_task(void){
	
	if(strcmp(rx_str, "INPUT 1 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 1;
		}else if (strcmp(rx_str, "INPUT 2 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 2;
		}else if (strcmp(rx_str, "INPUT 3 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 3;
		}else if (strcmp(rx_str, "INPUT 4 TEST")==0){
		
		input_flag = 4;
		}else if (strcmp(rx_str, "INPUT 5 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 5;
		}else if (strcmp(rx_str, "INPUT 6 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 6;
		}else if (strcmp(rx_str, "INPUT 7 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 7;
		}else if (strcmp(rx_str, "INPUT 8 TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 8;
		}else if (strcmp(rx_str, "KEY INPUT TEST")==0){
		gb_start_1_min_timer = true ;
		input_flag = 9;
	}
	
}

/*****************************************************************************
* Function name	: void Input_Test(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: The Input Test Process Function checks whether 
*				  the triggered input is correct or incorrect 
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Input_Test(void){
	
	if(gb_1_min_elapsed == true){
		if (input_flag != 0) {
			if ((input_flag != 1 && digInput.ip1_lth_f) ||
			(input_flag != 2 && digInput.ip2_lth_f) ||
			(input_flag != 3 && digInput.ip3_lth_f) ||
			(input_flag != 4 && digInput.ip4_lth_f) ||
			(input_flag != 5 && digInput.ip5_lth_f) ||
			(input_flag != 6 && digInput.ip6_lth_f) ||
			(input_flag != 7 && digInput.ip7_lth_f) ||
			(input_flag != 8 && digInput.ip8_lth_f) ||
			(input_flag != 9 && digInput.ip9_lth_f)) {
				if (!error_reported) {
					Print_Message("\nERROR: WRONG BUTTON PRESSED");
					error_reported = true;
				}
				return;
				} else {
				error_reported = false;
			}
		}
	}
	
		
	if ((input_flag == 1) && (digInput.ip1_lth_f == 1)) {
		Print_Message("\nINPUT 1 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
	}
	if (input_flag == 2 && digInput.ip2_lth_f == 1) {
		Print_Message("\nINPUT 2 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 3 && digInput.ip3_lth_f == 1) {
		Print_Message("\nINPUT 3 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 4 && digInput.ip4_lth_f == 1) {
		Print_Message("\nINPUT 4 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 5 && digInput.ip5_lth_f == 1) {
		Print_Message("\nINPUT 5 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 6 && digInput.ip6_lth_f == 1) {
		Print_Message("\nINPUT 6 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 7 && digInput.ip7_lth_f == 1) {
		Print_Message("\nINPUT 7 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 8 && digInput.ip8_lth_f == 1) {
		Print_Message("\nINPUT 8 TEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
	}
	if (input_flag == 9 && digInput.ip9_lth_f == 0) {
		Print_Message("\nKET PRESSED\nTEST OK");
		input_flag = 0;
		gb_start_1_min_timer = false ;
		
		}else {
		Error_task();
	}
	
	
}

/*****************************************************************************
* Function name	: void Error_task(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: To verify whether the triggered input or 
*				  RS485 communication is completed within the specified time.
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void Error_task(void){
	
	
	if(input_set_flag){
		if(input_flag == 1){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 1 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 2){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 2 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 3){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 3 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 4){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 4 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 5){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 5 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 6){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 6 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 7){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 7 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 8){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nINPUT 8 TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		if(input_flag == 9){
			Print_Message("\nINPUT TEST TIME OUT ,TEST ABORTED");
			Print_Message("\nKEY TEST NOT OK");
			input_flag = 0;
			input_set_flag = 0;
		}
		
		if(rx_485_flag){
			Print_Message("RS485 NOT OK\n");
			memset (rx_str, 0, 100);
			memset (rx_rs485_buff, 0, 100);
			input_set_flag = 0;
			rx_485_flag = 0;
		}
		if(rx_485_2_flag){
			Print_Message("RS485 2 NOT OK\n");
			memset (rx_str, 0, 100);
			memset (rx_rs485_2_buff, 0, 100);
			input_set_flag = 0;
			rx_485_2_flag = 0;
		}
	}
}

/*****************************************************************************
* Function name	: void relay_output(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: To check the output for relay
*				  
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
//void configure_led_pin(void) {
//	// Configure PA22 as output using ASF
//	ioport_set_pin_dir(PIO_PA22_IDX, IOPORT_DIR_OUTPUT); // Set PA22 as output
//	ioport_set_pin_level(PIO_PA22_IDX, false);           // Set PA22 to low (LED OFF initially)
//}


void relay_output(void){
	if(strcmp(rx_str, "TEST OUTPUT 1")==0){
		
		//ioport_set_pin_level(PIO_PA22_IDX, true);
		Operate_Relay(RELAY1,ON);
		Print_Message("\nOUTPUT1_HIGH");
		delay_ms(5000);
		//ioport_set_pin_level(PIO_PA22_IDX, false);
		Operate_Relay(RELAY1,OFF);
		Print_Message("\nOUTPUT1_LOW");
	}
	if(strcmp(rx_str, "TEST OUTPUT 2")==0){
		Operate_Relay(RELAY2,ON);
		Print_Message("\nOUTPUT2_HIGH");
		delay_ms(5000);
		Operate_Relay(RELAY2,OFF);
		Print_Message("\nOUTPUT2_LOW");
	}
	if(strcmp(rx_str, "TEST OUTPUT 3")==0){
		Operate_Relay(RELAY3,ON);
		Print_Message("\nOUTPUT3_HIGH");
		delay_ms(5000);
		Operate_Relay(RELAY3,OFF);
		Print_Message("\nOUTPUT3_LOW");
	}
	if(strcmp(rx_str, "TEST OUTPUT 4")==0){
		Operate_Relay(RELAY4,ON);
		Print_Message("\nOUTPUT4_HIGH");
		delay_ms(5000);
		Operate_Relay(RELAY4,OFF);
		Print_Message("\nOUTPUT4_LOW");
	}
	
	if(strcmp(rx_str, "TEST OUTPUT ALL")==0){
		Operate_Relay(ALL_RELAY,ON);
		Print_Message("\nOUTPUT_ALL_HIGH");
		delay_ms(5000);
		Operate_Relay(ALL_RELAY,OFF);
		Print_Message("\nOUTPUT_ALL_LOW");
	}
	
	if(strcmp(rx_str, "POWER LED TEST")==0){
		ON_RED_LED;
		Print_Message("\nPOWER LED IS ON");
		delay_ms(5000);
		OFF_RED_LED;
		Print_Message("\nPOWER LED IS OFF");
	}
	
	if(strcmp(rx_str, "RS485 1 LED TEST")==0){
		ON_YELLOW_LED;
		Print_Message("\nRS485 1 LED IS ON");
		Put_Data_On_RS485_UART("\nRS485 1 LED IS ON",19);
		delay_ms(5000);
		OFF_YELLOW_LED;
		Print_Message("\nRS485 1 LED IS OFF");
		Put_Data_On_RS485_UART("\nRS485 1 LED IS OFF",19);
	}
	
	if(strcmp(rx_str, "RS485 2 LED TEST")==0){
		ON_GREEN_LED;
		Print_Message("\nRS485 2 LED IS ON");
		Put_Data_On_RS485_UART("\nRS485 2 LED IS ON",19);
		delay_ms(5000);
		OFF_GREEN_LED;
		Print_Message("\nRS485 2 LED IS OFF");
		Put_Data_On_RS485_UART("\nRS485 2 LED IS OFF",19);
	}
	
}

/*****************************************************************************
* Function name	: void Real_Time_Clock(void)
* Returns		: Nothing.
* Arguments    	: NA
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: To test and Reset to specific time
*
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void Real_Time_Clock(void){
	if(strcmp(rx_str, "RTC TEST")==0){
		rtc_set_flag = 1;
		memset (rx_str, 0, 100);
	}
	if ((rtc_set_flag == 1) && (strncmp(rx_str, "SET RTC", 7) == 0)) {
		char *data = rx_str + 8; // Pointer to the date and time part
		char *token;
		token = strtok(data, "/");
		if (token != NULL) day = atoi(token);
		token = strtok(NULL, "/");
		if (token != NULL) month = atoi(token);
		token = strtok(NULL, " ");
		if (token != NULL) year = atoi(token);
		token = strtok(NULL, ":");
		if (token != NULL) hour = atoi(token);
		token = strtok(NULL, ":");
		if (token != NULL) minute = atoi(token);
		token = strtok(NULL, ":");
		if (token != NULL) second = atoi(token);
		Write_Time_hhMMss_To_DS1339A(hour, minute, second);
		Write_DtMnYy_and_Day_To_DS1339A(day, month, year, 0);
		delay_ms(1000);
		Print_Message("\n");
		Get_RTC_Data();
		Print_Message("RTC TEST OK\n");
		
		memset (rx_str, 0, 100);
		rtc_set_flag = 0;
	}
}

/*****************************************************************************
* Function name	: void Flash_Test(void)
* Returns		: Nothing.
* Arguments    	: NA
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: To test Flash Memory is Writing and Reading given Data 
*
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void Flash_Test(void){
	if(strcmp(rx_str, "FLASH TEST")==0){
		Print_Message("Writing Flash Test\n");
		Flash_Byte_Write(1,write_data, strlen((char *)write_data));
		delay_ms(100);
		Print_Message("FLASH WRITE OK\n");
		flash_flag++;
		Print_Message("\nReading Flash Test\n");
		Flash_Byte_Read(1,strlen((char *)write_data));
		delay_ms(100);
		if (strcmp(write_data, gb_fRead_Array) == 0){
			Print_Message("FLASH READ OK\n");
			flash_flag++;
		}
		if (flash_flag == 2){
			Print_Message("FLASH TEST OK\n");
			flash_flag = 0;
			}else{
			Print_Message("FLASH TEST NOT OK");
			flash_flag = 0;
		}
	}
}

/*****************************************************************************
* Function name	: void SRAM_Test(void)
* Returns		: Nothing.
* Arguments    	: NA
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: To test SRAM Memory is Writing and Reading given Data
*
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void SRAM_Test(void){
	if(strcmp(rx_str, "SRAM TEST")==0){
		Print_Message("Writing SRAM Test\n");
		RAM_MEM readMem;
		RAM_MEM writeMem;
		
		uint8_t writeString[] = "Hello, SRAM!";
		uint8_t strLength = strlen((char *)writeString);
		
		writeMem.regAdds = 0x000000;
		writeMem.data_len = strLength;
		memcpy(writeMem.data_arr, writeString, strLength);
		extRAM_Write_To_Memory(&writeMem);
		delay_ms(100);
		Print_Message("SRAM WRITE OK\n");
		sram_flag++;
		Print_Message("\nReading SRAM Test\n");
		
		readMem.regAdds = writeMem.regAdds;
		readMem.data_len = strLength;
		strcpy(readMem.data_arr, writeString);
		
		extRAM_Read_From_Memory(&readMem);

		if(strcmp(readMem.data_arr, writeString)==0){
			delay_ms(100);
			Print_Message("SRAM READ OK\n");
			sram_flag++;
		}
		if (sram_flag == 2){
			Print_Message("\nSRAM TEST OK\n");
			sram_flag = 0;
			}else{
			Print_Message("\nSRAM TEST FAILED\n");
			sram_flag = 0;
		}
	}
}

/*****************************************************************************
* Function name	: void EEPROM_Test(void)
* Returns		: Nothing.
* Arguments    	: NA
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: Read And Print MAC ID of EEPROM
*
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void EEPROM_Test(void){
	
	if(strcmp(rx_str, "MAC_EEPROM TEST")==0){
		U8 mac_id[6];
		char hex[10];
		EEPROM_EUI_READ(mac_id);
		Print_Message("MAC ID: ");
		for (int i = 0; i < 6; i++){
			Print_ASCII_HEX(mac_id[i]);
			if (i < 5){
				Print_Message(":");
			}
		}
		Print_Message("\n");
	}
}


/*****************************************************************************
* Function name	: void EEPROM_1_Test(void)
* Returns		: Nothing.
* Arguments    	: NA
* Created by	: Suryaprakash Parmeshwar Shendge.
* Date created	: 19/9/2025.
*
* Description	: Read And Write Operation For storing in external EEPROM
*
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void EEPROM_1_Test(void)
{
	if(strcmp(rx_str, "EEPROM TEST")==0)
	{
		U8 wr_arr[513] = {0};
		U8 rd_arr[513] = {0};
		
		bool lcl_eeprom_rd_flag = 0;
		bool lcl_eeprom_wr_flag = 0;

		// Fill the array using bitwise operations for alternation
		for (int i = 0; i < 512; i++)
		{
			wr_arr[i] = (i & 1) ? 0x55 : 0xAA;  // Alternate between 0xAA and 0x55
		}
		if(EEPROM_Page_Write(2592, 0, wr_arr, 512) == 0)
		{
			delay_ms(5000);
			lcl_eeprom_wr_flag = 1;
			Print_Message("\nEEPROM WRITE OK\n");
		}
		
		Print_Message("\nReading EEPROM Test  \n");
		
		if(EEPROM_Page_Read(2592, 0, rd_arr, 512) == 0)
		{
			delay_ms(5000);
			lcl_eeprom_rd_flag = 1 ;
			Print_Message("\nEEPROM READ OK\n");
		}

		if((strncmp(wr_arr,rd_arr,512) == 0) && (lcl_eeprom_wr_flag == 1) && (lcl_eeprom_rd_flag == 1))
		{
			delay_ms(5000);
			Print_Message("\nEEPROM TEST OK \n");
		}
		else
		{
			delay_ms(5000);
			Print_Message("\nEEPROM Test FAILED.\n");
		}
	}
	
}

/*****************************************************************************
* Function name	: RS_485_C1_Test(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: RS485 Channel 1 Communication test Process Function
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void RS_485_C1_Test(void){
	if(strcmp(rx_str, "RS485 1 TEST")==0){
		rx_485_flag = 1;
		Send_data_On_RS485("RS485 Channel 1 Receive",strlen("RS485 Channel 1 Receive"));
		memset (rx_str, 0, 100);
		memset (rx_rs485_buff, 0, 100);
	}
	
	if ((rx_485_flag) && (strcmp(rx_rs485_buff, "RS485 1 Command Transmit")==0))
	{
		Print_Message("\nRS485 1 Test OK\n");
		rx_485_flag = 0;
		input_set_flag = 0;
		memset (rx_rs485_buff, 0, 100);
	}
	else {
		Error_task();
	}
}

/*****************************************************************************
* Function name	: RS_485_C2_Task(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Anup Silvan Mascarenhas.
* Date created	: 20/12/2024.
*
* Description	: RS485 Channel 2 Communication test Process Function
*
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/

void RS_485_C2_Task(void){
	if(strcmp(rx_str, "RS485 2 TEST")==0){
		rx_485_2_flag = 1;
		Send_data_On_RS485("RS485 Channel 2 Receive",strlen("RS485 Channel 1 Receive"));
		memset (rx_str, 0, 100);
		memset (rx_rs485_2_buff, 0, 100);
	}
	
	if ((rx_485_2_flag) && (strcmp(rx_rs485_2_buff, "RS485 2 Command Transmit")==0))
	{
		Print_Message("\nRS485 2 Test OK\n");
		rx_485_2_flag = 0;
		input_set_flag = 0;
		memset (rx_rs485_2_buff, 0, 100);
	}
	else {
		Error_task();
	}
}
