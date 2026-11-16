/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: rs485_uart.c
* Created By	: Ranjitkumar Ainapure.
* Created Date	: 19/08/2023.
* Module
* Description	:	Initialization of RS485 UART, Definitions of respective functions
*					and setting up baud rate, parity, length of bits and interrupts.
*
* Controller	: 	ATSAM4E16CA-AUR
*					1024 KB		Flash.
*					128 KB		RAM.
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: 19/08/2023.
* Changes		: NA.
*****************************************************************************/
/***** System Includes *****/
#include "asf.h"

/***** User defined Includes *****/
#include "rs485_uart_2.h"
#include "user_uart.h"
//#include "definitions.h"

/***** Definations for RS485 uart initializations *****/
#define	RS485_BAUDRATE_2				(9600)
#define	RS485_CHAR_LEN_2				(US_MR_CHRL_8_BIT)
#define	RS485_PARITY_2				(US_MR_PAR_NO)
#define	RS485_STOP_BIT_2				(US_MR_NBSTOP_1_BIT)
/***** Definations for RS485 uart initializations *****/
#define USTX_BUF_LEN_2				(100)	/*Bytes*/


volatile U8 usart2_trans_buff_len = 0;	// Variable used to fill buffer length.
volatile U8 usart2_transmit_buff[USTX_BUF_LEN_2] = {0};	// Right now 100 characters are loaded in a buffer/array.
	
volatile U8 gb_usart2_ready_f = uSET;	// Flag used to check USART is ready or not for transmission.
volatile U8 gb_usart2_byte_rec_f = 0;	// Flag used to to set when a single byte received.
volatile U8 gb_usart2_rec_byte = 0;		// variable used to get a byte data from usart.

/***** Extern / Global Function declaration *****/
extern void RS485IntercharDelayLoad(void);
extern void Get_OSDP_Frame_Data(volatile uint8_t r_byte);

/*****************************************************************************
* Function name	: void RS485_UART_Init(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 19/08/2023.
*
*
* Description	:	Initialization of UART for RS485 communication.
* 					Baud rate is set at 9600 speed. length is 8bit, 1 stop bit.
* 					parity is none.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
//void RS485_UART2_Init(void)
//{
//	unsigned int lcl_error=0;
//	pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA21A_RXD1 | PIO_PA22A_TXD1);				//Allow USART0 to control PA21 and PA22
//	sysclk_enable_peripheral_clock(ID_USART1);											//Enable USART1 Clock
//	const sam_usart_opt_t usart1Settings =
//	{
//		RS485_BAUDRATE_2,
//		RS485_CHAR_LEN_2,
//		RS485_PARITY_2,
//		RS485_STOP_BIT_2
//	};	//USART1 Settings (configures desired baudrate and sets no parity)
//	lcl_error = usart_init_rs232(USART1, &usart1Settings, sysclk_get_peripheral_hz());			// Setup USART1 with configuration
//	if (lcl_error)																				// If any error go inside
//	{
//		while(1);																				// error
//	}
//	else
//	__NOP();
//	
// 	usart_enable_tx(USART1);	// Enable USART1 for tx
// 	//usart_enable_interrupt(USART1, US_IER_TXRDY);	// enable interrupt when 1 byte is filled in TX buffer
// 	usart_enable_rx(USART1);	// Enable USART1 for rx
// 	usart_enable_interrupt(USART1, US_IER_RXRDY);	// enable interrupt for Reception on one byte
//	NVIC_EnableIRQ(USART1_IRQn);	// Enable interrupt for USART1.
//	NVIC_SetPriority(USART1_IRQn, 2);	// Set priority level, lower the value highest in the priority.
//}

/*****************************************************************************
* Function name	: void Put_Data_On_RS485_UART(U8 *tdata, U8 tbuflen)
* Returns		: Nothing.
* Arguments    	: U8 *tdata ---> Holds address of buffer.
				  U8 tbuflen ---> pass buffer length to transmit data.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 19/08/2023.
*
* Description	:	Function to pass data over UART.
*
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Put_Data_On_RS485_UART2(U8 *tdata, U8 tbuflen)
{
	/***** Interrupt Method Transmit *****/
	U8 uidx = 0;
	while (tbuflen--)
	{
		usart2_transmit_buff[uidx] = *tdata;
		tdata ++;
		uidx ++;
	}
	usart2_trans_buff_len = uidx;
	usart_enable_interrupt(USART1, US_IER_TXRDY);	// Enable interrupt when 1 byte is filled in TX buffer
	gb_usart2_ready_f = uRESET;
}

/*****************************************************************************
* Function name	: void USART0_Handler(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 18/08/2023.
*
* Description	:	RS485 USART Handler when data receive on USART,
*					it reads a byte of data.
*
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void USART1_Handler(void)
{
	U32 Local_data = 0;
	static U8 tidx = 0;
	
	if ((usart_get_status(USART1) & US_CSR_TXRDY) == US_CSR_TXRDY)	// If transmitter is ready?
	{
		if (usart2_trans_buff_len > 0)
		{
			usart_write(USART1, usart2_transmit_buff[tidx]);
			usart2_trans_buff_len --;
			tidx ++;
		}
		else
		{
			__NOP();
		}
	}
	else
	{
		__NOP();
	}
	
	if ((usart_get_status(USART1) & US_CSR_TXEMPTY) == US_CSR_TXEMPTY)
	{
		tidx = 0;
		gb_usart2_ready_f = uSET;
		usart_disable_interrupt(USART1, US_IDR_TXRDY);
	}
	else
	{
		__NOP();
	}
	
	if ((usart_get_status(USART1) & US_CSR_RXRDY) == US_CSR_RXRDY)	// If data received.
	{
		usart_read(USART1, &Local_data);	/* Read a byte. */
		gb_usart2_byte_rec_f = uSET;			/* Set this flag once received a single byte. */
		gb_usart2_rec_byte = Local_data;		/* Copy a byte to a global variable. */
		//Get_OSDP_Frame_Data(Local_data);	/* Pass a received byte to decode further. */
		RS485IntercharDelayLoad();			/* Fill inter character delay, after a byte received. */
	}
	else
	{
		__NOP();
	}
}