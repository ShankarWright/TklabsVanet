/**
 *	@file	tests.c
 *
 *	@brief	Test routines code
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <asf.h>
#include "tests.h"
#include "vanet.h"


typedef struct 
{
	const char *title;
	void (*test)(void);
	bool run_always;
	bool ask_to_run;
	bool pause_after;
	bool enable_test;
} db_tests_t;

db_tests_t test_cases[] = 
{
/* Title                Test Routine                Run     Ask     Pause   Enabled */
{ "Reset",              reset_do_soft_reset,        false,  false,  false,	true  },
{ "RTC Test",			rtc_enable_tick,			true,	false,	false,  false },	// always run this
{ "Memory Test",		memory_test,				false,	false,	false,	true  },
{ "GPS NMEA",			gps_nmea,					false,  false,  false,	true  },
{ "TWIM Discovery",		twim_discover_devices,		true,	false,	false,	true  },
{ "MPU-6500 Basic",     mpu_6500_basic,             false,  false,  false,  true  },
{ "LIS3DSH Basic",      lis3dsh_basic,              false,  false,  false,  true  },
{ "PCA9501 EEPROM",		i2c_pca9501_test_eeprom,    false,  false,  false,  false },
{ "I2C LCD Line/Pos",	i2c_lcd_test_line_pos,		false,	false,	false,  false },
{ "I2C LCD Moving",		i2c_lcd_test_moving,		false,	false,	false,  false },
{ "I2C LCD Basic",		i2c_lcd_basic,				false,	false,	false,  false },
{ "Piezo Bit-Bang",		piezo_bit_bang,				true,	false,	false,	true  },
{ "I2C Abstract Class",	i2c_abstract_class,			false,	false,  false,  false },
{ "ALIX Hello World",	alix_comm,					false,	false,	false,	true  },
#ifdef BSP_CONFIG_UCOS
{ "GSM27010 UE Test",	mux_test,					false,	false,	false,	true  },
#endif
{ "DS1077 Test",        ds1077_test,				false,  false,  false,  false },
{ "MAX17043 Test",      max17043_test,				false,	false,	false,  false },
{ "GPIO/INTC Test",		pin_gpio_intc_test,			false,  false,  false,  true  },
{ "USART ISR Test",     usart_isr_test,             false,  false,  false,  true  }
};

#define NUM_TEST_CASES (sizeof(test_cases)/sizeof(db_tests_t))

void run_tests(void)
{
	int get;

	for (int i=0; i<NUM_TEST_CASES; i++)
	{
		if (test_cases[i].run_always)
		{	
			print_dbg("Running Test [");
			print_dbg(test_cases[i].title);
			print_dbg("]\r\n");
			test_cases[i].test();
			print_dbg("Test Complete\r\n\r\n");
		}
		else if (test_cases[i].ask_to_run)
		{
			print_dbg("Run Test ");
			print_dbg(test_cases[i].title);
			print_dbg(" ...?");
			get = usart_getchar(DBG_USART);
			if (get == 'y' || get == 'Y')
			{
				test_cases[i].test();
				print_dbg("Test Complete\r\n");
			}				
		}						
			
		if (test_cases[i].pause_after)
		{
			print_dbg("press any key...\r\n\r\n");
			usart_getchar(DBG_USART);
		}		
		
	}
}

void show_test_menu(void)
{
	int ch;
	char menuitem[2];
	menuitem[1] = '\0';
	
	print_dbg("\r\nTest Menu:\r\n");
	for (int i=0; i<NUM_TEST_CASES; i++)
	{
		if (test_cases[i].enable_test)
		{
			menuitem[0] = 'a' + i;
			print_dbg(menuitem);
			print_dbg(" - ");
			print_dbg(test_cases[i].title);
			print_dbg("\r\n");
		}
	}
	print_dbg("> ");
	ch = usart_getchar(DBG_USART) - 'a';
	if (ch < NUM_TEST_CASES)
	{
		if (test_cases[ch].enable_test)
		{
			// run test
			print_dbg("\r\nRunning Test [");
			print_dbg(test_cases[ch].title);
			print_dbg("]\r\n");
			test_cases[ch].test();
			print_dbg("Test Complete\r\n\r\n");
		}
	}
	else
	{
		// eat another char
		ch = usart_getchar(DBG_USART);
	}
}