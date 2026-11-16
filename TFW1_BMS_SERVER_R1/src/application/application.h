/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: applicaion.h
* Created By	: Anup Silvan Mascarenhas.
* Created Date	: 20/12/2024.
* Module
* Description	: Header file for applicaion.c
				  Defines constants and macros for applicaion.c.
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: 05/10/2023.
* Changes		: NO.
*****************************************************************************/
#ifndef APPLICATION_H_
#define APPLICATION_H_

extern volatile uint8_t tc_flag;			//Flag used to to set for time counter.
extern volatile char rx_str[100];			//Receiving string buffer  
extern volatile uint8_t ch_i;				//Variable index for transmitting and receiving buffer 
extern volatile uint8_t rx_flag;			//Flag used to set UART receives the buffer string

extern volatile uint8_t input_set_flag;		//	Variable input set flag for different inputs 
extern volatile uint8_t input_flag ;		//	Flag when any input is set 
extern volatile bool error_reported;		//	Flag to sets when wrong input detected

extern volatile uint8_t rtc_set_flag;		//	Flag used to set RTC Time and Date 
extern volatile uint8_t day, month, year, hour, minute, second;

extern volatile uint8_t write_data[];		//	Buffer used write flash Memory
extern volatile uint8_t flash_flag;			//	Flag used to set set when read and write is success
extern volatile uint8_t sram_flag;			//	Flag used to set set when read and write is success

extern volatile uint8_t rs_i;				//	Variable index for transmitting and receiving buffer
extern volatile char rx_rs485_buff[100];	//	Receiving string buffer for RS485 channel 1
extern volatile uint8_t rs485_flag;			//	Flag used to set when UART Communicates with RS485 channel 1
extern volatile uint8_t rx_485_flag;		//	Flag used to set RS485 receives the buffer string

extern volatile uint8_t rs_2_i;				//	Variable index for transmitting and receiving buffer
extern volatile char rx_rs485_2_buff[100];	//	Receiving string buffer for RS485 channel 1
extern volatile uint8_t rs485_2_flag;		//	Flag used to set when UART Communicates with RS485 channel 1
extern volatile uint8_t rx_485_2_flag;		//	Flag used to set RS485 receives the buffer string

/***** Function Declarations / Prototypes *****/
void configure_console(void);
void Test_Func_Init(void);
void Pheriheral_test_func(void);
void UART_Task(void);
void Error_task(void);
void configure_led_pin(void);
void Digial_input_task(void);
void Input_Test(void);
void relay_output(void);
void Real_Time_Clock(void);
void configure_twi(void);
void Flash_Test(void);
void SRAM_Test(void);
void EEPROM_Test(void);
void EEPROM_1_Test(void);
void RS_485_C1_Test(void);
void RS_485_C2_Task(void);

#endif /* APPLICATION_H_ */