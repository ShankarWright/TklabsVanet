/**
 *	@file	pwm.c
 *
 *	@brief	PWM Driver
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */


#include <asf.h>
#include <string.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_PWM

static uint32_t period;


#ifdef CONFIG_STI_CMD_PWM
static void pwm_handler(int argc, char** argv, uint8_t port)
{
	bool ret;
	if (!strncmp(argv[1], "dis", 3))
	{
		bsp_pwm_disable_channel(2);
	}
	else if (argc == 3 && !strncmp(argv[1], "en", 2))
	{
		int freq = strtol(argv[2],0,10);
		bsp_termios_printf(port, "Enabling PWM with Frequency %dHz\r\n", freq);
		ret = bsp_pwm_enable_channel(2, freq, 50);
		if (!ret)
		{
			bsp_termios_printf(port, "\r\nError setting PWM - Check logcat\r\n");
		}
	}
	else if (argc == 3 && !strncmp(argv[1], "duty", 2))
	{
		int duty = strtol(argv[2],0,10);
		bsp_pwm_update(2, duty);
	}
}

static bsp_sti_command_t pwm_command =
{
	.name = "pwm",
	.handler = &pwm_handler,
	.minArgs = 1,
	.maxArgs = 2,
	STI_HELP("pwm [en freq | dis | duty pct]        PWM Configuration")
};

#endif // CONFIG_STI_CMD_PWM

void bsp_pwm_init(void)
{
	// PWM controller configuration.
	pwm_opt_t pwm_opt;
		
	pwm_opt.diva = AVR32_PWM_DIVA_CLK_OFF;
	pwm_opt.divb = AVR32_PWM_DIVB_CLK_OFF;
	pwm_opt.prea = AVR32_PWM_PREA_CCK;
	pwm_opt.preb = AVR32_PWM_PREB_CCK;

	pwm_opt.fault_detection_activated = false;
	pwm_opt.sync_channel_activated    = false;
	pwm_opt.sync_update_channel_mode  = PWM_SYNC_UPDATE_MANUAL_WRITE_MANUAL_UPDATE;
	pwm_opt.sync_channel_select[0]    = true;
	pwm_opt.sync_channel_select[1]    = true;
	pwm_opt.sync_channel_select[2]    = false;
	pwm_opt.sync_channel_select[3]    = false;
	pwm_opt.cksel                     = PWM_CKSEL_MCK;	// module clock
	pwm_init(&pwm_opt);
		
	// Update the period
	pwm_update_period_value(10);
	
	#ifdef CONFIG_STI_CMD_PWM
	// install our STI command
	bsp_sti_register_command(&pwm_command);
	#endif // CONFIG_STI_CMD_PWM
}

bool bsp_pwm_enable_channel(uint8_t channel, uint32_t frequency, uint8_t duty)
{
	const uint8_t min_period = 200;
	uint32_t mck;
	uint16_t prescaler;
	uint8_t cpre;
	avr32_pwm_channel_t pwm_channel = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};
	
	// this need to be configurable... but for now... hard code pwm channel 2 on main pins
	if (channel != 2)
	{
		bsp_logcat_printf(BSP_LOGCAT_WARNING, "PWM: Unknown Channel");
		return false;
	}		
		
	if (duty > 100)
		duty = 100;
		
	// make sure frequency is attainable, we start with period=100 to maximize frequency high end
	period = min_period;
	mck = sysclk_get_peripheral_bus_hz(&AVR32_PWM);
	prescaler = mck / frequency / period;
	cpre = 255;
	if (prescaler < 2)
		cpre = AVR32_PWM_CPRE_CCK;
	else if (prescaler < 4)
		cpre = AVR32_PWM_CPRE_CCK_DIV_2;
	else if (prescaler < 8)
		cpre = AVR32_PWM_CPRE_CCK_DIV_4;
	else if (prescaler < 16)
		cpre = AVR32_PWM_CPRE_CCK_DIV_8;
	else if (prescaler < 32)
		cpre = AVR32_PWM_CPRE_CCK_DIV_16;
	else if (prescaler < 64)
		cpre = AVR32_PWM_CPRE_CCK_DIV_32;
	else if (prescaler < 128)
		cpre = AVR32_PWM_CPRE_CCK_DIV_64;
	else if (prescaler < 256)
		cpre = AVR32_PWM_CPRE_CCK_DIV_128;
	else if (prescaler < 512)
		cpre = AVR32_PWM_CPRE_CCK_DIV_256;
	else if (prescaler < 1024)
		cpre = AVR32_PWM_CPRE_CCK_DIV_512;
	else
		cpre = AVR32_PWM_CPRE_CCK_DIV_1024;			// just use the maximum prescaler and let period be bigger
	
	bsp_logcat_printf(BSP_LOGCAT_DEBUG, "Desired prescaler %d, picked %d", prescaler, (1<<cpre));
	
	// Now recalc the period based on a usable prescaler
	period = mck / frequency / (1 << cpre);
	
	if (period < min_period)
	{
		bsp_logcat_printf(BSP_LOGCAT_WARNING, "PWM: Requested frequency too high");
		return false;
	}
	
	// configure GPIO
	gpio_enable_module_pin(AVR32_PWM_PWML_2_PIN, AVR32_PWM_PWML_2_FUNCTION);
	gpio_enable_module_pin(AVR32_PWM_PWMH_2_PIN, AVR32_PWM_PWMH_2_FUNCTION);
	
	// Channel configuration
	pwm_channel.CMR.dte   = 0;						// Enable Deadtime for complementary Mode
	pwm_channel.CMR.dthi  = 1;						// Deadtime Inverted on PWMH
	pwm_channel.CMR.dtli  = 1;						// Deadtime Not Inverted on PWML
	pwm_channel.CMR.ces   = 0;						// 0/1 Channel Event at the End of PWM Period
	pwm_channel.CMR.calg  = PWM_MODE_LEFT_ALIGNED;	// Channel mode.
	pwm_channel.CMR.cpol  = PWM_POLARITY_LOW;       // Channel polarity.
		
	// (MCK/prescaler)/period = PWM freq
	pwm_channel.CMR.cpre  = cpre;					// Channel prescaler.
	pwm_channel.cdty      = duty / 100.0 * period;	// Channel duty cycle, should be < CPRD.
	pwm_channel.cprd      = period;					// Channel period.
	
	bsp_logcat_printf(BSP_LOGCAT_DEBUG, "PWM duty / period = %d / %d", pwm_channel.cdty, pwm_channel.cprd);
	pwm_channel_init(channel, &pwm_channel);
	pwm_start_channels((1 << channel));
	
	return true;
}

bool bsp_pwm_disable_channel(uint8_t channel)
{
	// this need to be configurable... but for now... hard code pwm channel 2 on main pins
	if (channel != 2)
	{
		bsp_logcat_printf(BSP_LOGCAT_WARNING, "PWM: Unknown Channel");
		return false;
	}
	
	// Stop PWM
	pwm_stop_channels((1 << channel));
	    
	// move control back to GPIO
	gpio_enable_gpio_pin(AVR32_PWM_PWML_2_PIN);
	gpio_enable_gpio_pin(AVR32_PWM_PWMH_2_PIN);
	
	return true;
}

bool bsp_pwm_update(uint8_t channel, uint8_t new_duty)
{
	// this need to be configurable... but for now... hard code pwm channel 2 on main pins
	if (channel != 2)
	{
		bsp_logcat_printf(BSP_LOGCAT_WARNING, "PWM: Unknown Channel");
		return false;
	}
	
	avr32_pwm_channel_t pwm_channel;
	   
	// Update Duty
	pwm_channel.cdtyupd = new_duty / 100.0 * period;
	pwm_channel.cprdupd = period;
	pwm_update_channel(channel, &pwm_channel);
	
	bsp_logcat_printf(BSP_LOGCAT_DEBUG, "PWM duty / period = %d / %d", pwm_channel.cdtyupd, pwm_channel.cprdupd);
	
	return true;
}
#endif // CONFIG_BSP_ENABLE_PWM