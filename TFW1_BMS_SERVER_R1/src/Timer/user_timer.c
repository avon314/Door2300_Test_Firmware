/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: user_timer.c
* Created By	: Ranjitkumar Ainapure.
* Created Date	: 30/10/2022.
* Module
* Description	:	Initialization of timer module, ISR initialized for 1 ms.
*
* Controller	: 	ATSAM4S8B
*					512 KB		Flash.
*					128 KB		RAM.
*
* REVISION HISTORY
* Version		: 1.0
* Revision Date	: 30/10/2022.
* Changes		: NA.
*****************************************************************************/
#include "asf.h"
#include "defination.h"
#include "application.h"
#include "rs485_driver_2.h"
#include "rs485_uart_2.h"

/***** Local variables *****/
volatile uint16_t ms = 0;
volatile uint16_t sec = 0 , tsec = 0 ,jsec = 0;


/***** Global variables *****/
volatile uint8_t gb_timer_f = 0;
volatile U16 delay_counts = 0;			// Variable used to generate delay using timer interrupt.

extern U16 gb_srh_count;
extern U8 gb_tm_cnt_f;

bool gb_timer = false ;

bool gb_start_1_min_timer = false ;
bool gb_1_min_elapsed = false ;

bool gb_start_timer = false ;
bool gb_boot_jump_start_timer = false ;
/***** Extern Function Declarations *****/
extern void RS485IntercharDelayError(void);
extern void Clear_RS485_UART_Flags(void);
extern void OSDP_Poll_Delay(void);
extern void OSDP_Frame_Response_Time(void);
extern void Interlock_Time_Delay(void);
/*****************************************************************************
* Function name	: void Timer_Init(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 30/10/2022.
*
*
* Description	:	Initialization of timer 0 module. and interrupt generation
*					at every 1 ms.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Timer_Init(void)
{
	tc_enable_interrupt(TC1, 0, TC_IER_CPCS);	//	Enable channel 0 timer interrupt on compare match value for TC1
	Timer_config(1000);
}

/*****************************************************************************
* Function name	: void Timer_config(uint32_t freq_desired1)
* Returns		: Nothing.
* Arguments    	: uint32_t freq_desired1 ---> Pass desired frequency.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 30/10/2022.
*
*
* Description	:	Configuration of timer 0 module and interrupt priority of that.
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void Timer_config(uint32_t freq_desired1)
{
	uint32_t counts1;
	uint32_t ul_div1;
	uint32_t ul_tcclks1;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();			// read Master clock and store into local var

	pmc_enable_periph_clk(ID_TC3);						// enable Peripheral  clock for TC1

	// Configure TC for a desired frequency and trigger on RC compare on channel 1.
	tc_find_mck_divisor(
	(uint32_t)freq_desired1,							// The desired frequency as a uint32.
	ul_sysclk,											// Master clock freq in Hz.
	&ul_div1,											// Pointer to register where divisor will be stored.
	&ul_tcclks1,										// Pointer to reg where clock selection number is stored.
	ul_sysclk);											// Board clock freq in Hz.

	tc_init(TC1, 0, ul_tcclks1 | TC_CMR_CPCTRG);			// initialize TC for 1msec delay


	// Find the best estimate of counts, then write it to TC register C channel 0.
	counts1 = (ul_sysclk/ul_div1)/freq_desired1;		// calculate count for TC register c channel0
	tc_write_rc(TC1, 0, counts1);

	// Enable interrupts for this TC, and start the TC.
	tc_enable_interrupt(TC1, 0, TC_IER_CPCS);			// Enable interrupt for TC1 compare capture.

	tc_start(TC1, 0);									// Start TC1.
	irq_register_handler(TC3_IRQn, 1);
}

/*****************************************************************************
* Function name	: void Configure_WDT(void)
* Returns		: void.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 01/04/2023.
*
*
* Description	:	Configure WDT. for 3 seconds.
					And set reset time to 2 seconds.
*               :
* Notes			: NA
* Global Variables Affected : gb_delay_time.
*****************************************************************************/
void Configure_WDT(void)
{
 	uint32_t wdt_mode;
 	U32 timeout_value;
 	
 	/* Get timeout value. */
 	timeout_value = wdt_get_timeout_value(WDT_PERIOD * 1000, BOARD_FREQ_SLCK_XTAL);
 	//Print_Number(timeout_value);
 	if (timeout_value == WDT_INVALID_ARGUMENT)
 	{
 		while (1)
 		{
 			/* Invalid timeout value, error. */
 			//printf("Invalid Timeout Value.\n");
 		}
 	}
 	
 	/* Configure WDT to trigger an interrupt (or reset). */
 	wdt_mode = WDT_MR_WDFIEN |  /* Enable WDT fault interrupt. */
 	WDT_MR_WDRSTEN |			/* Watchdog Reset Enable */
 	WDT_MR_WDRPROC;			/* WDT fault resets processor only. */
 	//WDT_MR_WDDBGHLT  |  /* WDT stops in debug state. */
 	//WDT_MR_WDIDLEHLT; /* WDT stops in idle state. */
 	
 	/* Initialize WDT with the given parameters. */
 	wdt_init(WDT, wdt_mode, timeout_value, timeout_value);
 
 	/* Configure and enable WDT interrupt. */
 	NVIC_DisableIRQ(WDT_IRQn);
 	NVIC_ClearPendingIRQ(WDT_IRQn);
 	NVIC_SetPriority(WDT_IRQn, 0);
 	NVIC_EnableIRQ(WDT_IRQn);
}

/*****************************************************************************
* Function name	: void WDT_Handler(void)
* Returns		: Nothing.
* Arguments    	: void.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 01/04/2023.
*
*
* Description	:	Interrupt service routine of WDT (Watchdog).
*               :
* Notes			: NA
* Global Variables Affected : NA.
*****************************************************************************/
void WDT_Handler(void)
{
	/* Clear status bit to acknowledge interrupt by dummy read. */
	uint32_t val= wdt_get_status(WDT);
	/* Restart the WDT counter. */
	//wdt_restart(WDT);
}

/*****************************************************************************
* Function name	: void timer_delay(volatile U16 del)
* Returns		: Nothing.
* Arguments    	: volatile U16 del ---> Pass delay timer value in ms.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 03/02/2023.
*
*
* Description	: Timer interrupt base delay will be generated by this function.
*               :
* Notes			: NA
* Global Variables Affected : NA
*****************************************************************************/
void timer_delay(volatile U16 del)
{
	delay_counts = del;
	while (delay_counts);
}

/*****************************************************************************
* Function name	: void Decrement_Delay_Counts(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 30/10/2022.
* Description	: Decrements the delay counts.
*               :
* Notes			: Call in 1ms of timer ISR.
* Global Variables Affected : NA.
*****************************************************************************/
void Decrement_Delay_Counts(void)
{
	if (delay_counts > 0)
	{
		delay_counts --;
	}
}

/*****************************************************************************
* Function name	: void Generate_General_Delay(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 30/10/2022.
* Description	: Function generates 1 secs of delay.
*               :
* Notes			: Call in 1ms of timer ISR.
* Global Variables Affected : gb_timer_f ---> flag sets evry 1 sec.
*****************************************************************************/
void Generate_General_Delay(void)
{
	ms ++;
	if (ms >= 1000)
	{
		ms = 0;
		if(gb_start_timer)
		{
			tsec ++ ;
		}
		if(gb_start_1_min_timer)
		{
			sec ++ ;
		}
		if(gb_boot_jump_start_timer)
		{
			jsec++ ;
		}
		gb_timer_f = true;

	}
	if(tsec >= 60 )
	{
		gb_timer = true;
		tsec = 0 ;
	}
	
	if(sec >= 60 )
	{
		gb_1_min_elapsed = true;
		sec = 0 ;
	}
}

/*****************************************************************************
* Function name	: void TC0_Handler(void)
* Returns		: Nothing.
* Arguments    	: None.
* Created by	: Ranjitkumar Ainapure.
* Date created	: 30/10/2022.
*
*
* Description	:	Timer 0 interrupt service routine.
*               :
* Notes			: NA
* Global Variables Affected : gb_timer_f ---> flag sets evry 1 sec.
							  gb_captone_f ---> Flag used to generate tone for capsense.
							  gb_poll_time ---> Decrement poll time. If sets zero fills with defined poll time.
							  gb_poll_flag ---> sets the flag after poll time reaches zero.
*****************************************************************************/
void TC3_Handler(void)
{
	volatile uint32_t status = tc_get_status(TC1, 0);
	if ((status & TC_SR_CPCS) == TC_SR_CPCS)
	{

		static volatile uint8_t busy_flag = 0;
		static volatile uint8_t tc = 0;
		static uint8_t rs_tc = 0;
		static uint8_t rs_2_tc = 0;
		static volatile uint16_t input_tc = 0;
		
		//RS485IntercharDelayError();	/* Flag sets if inter character delay exceeds. */
		//Clear_RS485_UART_Flags();	/* Clear flags related RS485 UART receive. */
		Generate_General_Delay();	/* General 1 sec of timer flag set. */
		//Decrement_Delay_Counts();	/* Decrements delay counts */
		
		
//----------------------------UART channel timer------------------------------------*/		
		if(gb_uart_byte_rec_f == uSET){
			tc = 0;
			gb_uart_byte_rec_f = uRESET;
			rx_str[ch_i++] = gb_uart_rec_byte;
		}else{
			tc++;
			if(tc >= 10){
				tc = 0;
				//rx_str[ch_i++] = 0 ;
				if(ch_i > 0)
				{
					tc_flag = 1;
				}
				
			}
		}
		
		
//----------------------------Timeout Channel------------------------------------*/		
		if(input_flag || rx_485_flag || rx_485_2_flag){            
			input_tc++;
			if(input_tc >= 1000*30){
				input_set_flag = 1;
				input_tc = 0;
			}
		}
		
		
//----------------------------RS485 channel 1 Timer------------------------------------*/		
		if (gb_usart_byte_rec_f == uSET){
			 rs_tc = 0;                     
			 gb_usart_byte_rec_f = uRESET; 
			 rx_rs485_buff[rs_i++] = gb_usart_rec_byte; 
		 }
		 else
		 {
			 rs_tc++; 
			 if (rs_tc >= 10) 
			 {
				 rs_tc = 0;    
				 rs485_flag = 1; 
			 }
		 }

		 

		 
//----------------------------RS485 channel 2 Timer------------------------------------*/		 
		 if (gb_usart2_byte_rec_f == uSET){
			 rs_2_tc = 0;
			 gb_usart2_byte_rec_f = uRESET;
			 rx_rs485_2_buff[rs_2_i++] = gb_usart2_rec_byte;
		 }
		 else
		 {
			 rs_2_tc++;
			 if (rs_2_tc >= 10)
			 {
				 rs_2_tc = 0;
				 rs485_2_flag = 1;
			 }
		
		}
	}
}