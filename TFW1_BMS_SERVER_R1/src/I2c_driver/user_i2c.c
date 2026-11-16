 /*
 * Module Name	: user_i2c.c
*/
 
 
 
#include "asf.h"
#include "user_i2c.h"
#include "user_uart.h"

void configure_twi(void)
{
	pmc_enable_periph_clk(ID_TWI0);

	// Configure SCL and SDA pins for TWI functionality
	pio_configure(PIOA, PIO_PERIPH_A, PIO_PA3A_TWD0 | PIO_PA4A_TWCK0, PIO_PULLUP);
	
	/* Initialize TWI driver with I2C bus clock set to 100kHz. */
	twi_options_t opt;
	opt.master_clk = sysclk_get_cpu_hz();   //cpu
	opt.speed = 100000;
	if(twi_master_setup(TWI0, &opt) != TWI_SUCCESS)
	{
		Print_Message("\nTWI initialization failed.");
	}
}