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


/************************************************************************/
/* RTC Tests                                                            */
/************************************************************************/
void rtc_enable_tick(void)
{
	bsp_rtc_init();
	//gpio_configure_pin(BSP_INIT_RTC_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
}

/************************************************************************/
/* Memory Tests                                                         */
/************************************************************************/
void memory_test(void)
{
	print_dbg("Heap Current/Total Used Size: ");
	print_dbg_ulong(get_heap_curr_used_size());
	print_dbg(" / ");
	print_dbg_ulong(get_heap_total_used_size());
	print_dbg("\r\n");
	
	void *ptr = malloc(1024);
	print_dbg("Malloc 1K: ptr = ");
	print_dbg_hex((unsigned long)ptr);
	print_dbg("\r\n");
	
	print_dbg("Heap Current/Total Used Size: ");
	print_dbg_ulong(get_heap_curr_used_size());
	print_dbg(" / ");
	print_dbg_ulong(get_heap_total_used_size());
	print_dbg("\r\n");
	
	print_dbg("Free 1K\r\n");
	free(ptr);
	
	print_dbg("Heap Current/Total Used Size: ");
	print_dbg_ulong(get_heap_curr_used_size());
	print_dbg(" / ");
	print_dbg_ulong(get_heap_total_used_size());
	print_dbg("\r\n");
}


/************************************************************************/
/* GPS                                                                  */
/************************************************************************/
void gps_nmea(void)
{
	// initialize UART0 @ 9600 bps
	static const gpio_map_t USART0_GPIO_MAP =
	{
	{AVR32_USART0_RXD_PIN, AVR32_USART0_RXD_FUNCTION},
	{AVR32_USART0_TXD_PIN, AVR32_USART0_TXD_FUNCTION}
	};

	usart_options_t usart0_options =
	{
		.baudrate = 9600,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	// Setup GPIO for USART0
	gpio_enable_module(USART0_GPIO_MAP,
		sizeof(USART0_GPIO_MAP) / sizeof(USART0_GPIO_MAP[0]));

	// Initialize it in RS232 mode.
	usart_init_rs232(&AVR32_USART0, &usart0_options, sysclk_get_pba_hz());
	
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		// echo usart0 to debug port
		if (usart_read_char(&AVR32_USART0, &ch) != USART_RX_EMPTY)
			usart_write_char(DBG_USART, ch);
	}
}

/************************************************************************/
/* Piezo                                                                */
/************************************************************************/
static void piezo_bit_bang_freq_dur(int dur_ms, int freq_hz)
{
	int period_us = 1000000 / freq_hz;
	int dur_loops = dur_ms * 1000 / period_us;
	
	for (int i=0; i<dur_loops; i++)
	{
		gpio_set_pin_high(AVR32_PIN_PB19);
		gpio_set_pin_low(AVR32_PIN_PB20);
		cpu_delay_us(period_us / 2, sysclk_get_cpu_hz());
		gpio_set_pin_low(AVR32_PIN_PB19);
		gpio_set_pin_high(AVR32_PIN_PB20);
		cpu_delay_us(period_us / 2, sysclk_get_cpu_hz());
	}
}

void piezo_bit_bang(void)
{
	gpio_configure_pin(AVR32_PIN_PB19, GPIO_DIR_OUTPUT);
	gpio_configure_pin(AVR32_PIN_PB20, GPIO_DIR_OUTPUT);
	
	piezo_bit_bang_freq_dur(200, 392);
	piezo_bit_bang_freq_dur(250, 660);
	piezo_bit_bang_freq_dur(200, 523);
	
	gpio_configure_pin(AVR32_PIN_PB19, GPIO_DIR_INPUT);
	gpio_configure_pin(AVR32_PIN_PB20, GPIO_DIR_INPUT);
}

/************************************************************************/
/* Mainboard Comm                                                       */
/************************************************************************/
void alix_comm(void)
{
	// initialize UART2 @ 115200 bps
	static const gpio_map_t USART2_GPIO_MAP =
	{
	{AVR32_USART2_RXD_1_PIN, AVR32_USART2_RXD_1_FUNCTION},
	{AVR32_USART2_TXD_1_PIN, AVR32_USART2_TXD_1_FUNCTION}
	};

	usart_options_t usart2_options =
	{
		.baudrate = 115200,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};
	
	// Setup GPIO for USART2
	gpio_enable_module(USART2_GPIO_MAP,
	sizeof(USART2_GPIO_MAP) / sizeof(USART2_GPIO_MAP[0]));

	// Initialize it in RS232 mode.
	usart_init_rs232(&AVR32_USART2, &usart2_options, sysclk_get_pba_hz());
	
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		print(&AVR32_USART2, "Hello Alix\r\n");
	}
}

#ifdef BSP_CONFIG_UCOS
void mux_test(void)
{
	mux_init();
	
	int ch;
	print_dbg("The mux will run until you hit a key...\r\n");
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		mux_run();
	}
}
#endif


static uint32_t s_gpio_pin = AVR32_PIN_PB02;

static void pin_int_handler(void)
{
	gpio_clear_pin_interrupt_flag(s_gpio_pin);
	print_dbg("pin!\r\n");
}

void pin_gpio_intc_test(void)
{
	INTC_register_GPIO_interrupt(&pin_int_handler, s_gpio_pin);
	gpio_enable_gpio_pin(s_gpio_pin);
	gpio_enable_pin_glitch_filter(s_gpio_pin);
	gpio_enable_pin_pull_up(s_gpio_pin);
	gpio_enable_pin_interrupt(s_gpio_pin, GPIO_FALLING_EDGE);
}


uint8_t usart_rx_buf[128];
uint8_t usart_tx_buf[128];
bsp_circ_buffer_t usart_rx;
bsp_circ_buffer_t usart_tx;

BSP_INT_ATTR static void usart_isr_handler(void)
{
	int c;
	
	if (MISC_USART->csr & AVR32_USART_CSR_RXRDY_MASK)
	{
		if (usart_read_char(MISC_USART, &c) == USART_RX_ERROR)
		{
			usart_reset_status(MISC_USART);
			return;
		}
		else
		{
			bsp_circ_writeb(&usart_rx, c);
		}
	}
	
	if (MISC_USART->csr & AVR32_USART_CSR_TXRDY_MASK)
	{
		c = bsp_circ_readb(&usart_tx);
		if (c > 0)
		{
			MISC_USART->thr = (c << AVR32_USART_THR_TXCHR_OFFSET) & AVR32_USART_THR_TXCHR_MASK;
		}
		else
		{
			MISC_USART->idr = AVR32_USART_IDR_TXRDY_MASK;
		}
	}
}

void usart_isr_test(void)
{
	// initialize UART1 @ 115200 bps
	static const gpio_map_t USART1_GPIO_MAP =
	{
		{MISC_USART_RX_PIN, MISC_USART_RX_FUNCTION}, {MISC_USART_TX_PIN, MISC_USART_TX_FUNCTION}
	};

	usart_options_t usart1_options =
	{
		.baudrate = 115200,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};
	
	// Setup GPIO for USART1
	gpio_enable_module(USART1_GPIO_MAP,
		sizeof(USART1_GPIO_MAP) / sizeof(USART1_GPIO_MAP[0]));

	// Initialize it in RS232 mode.
	usart_init_rs232(MISC_USART, &usart1_options, sysclk_get_pbc_hz());
	
	// Our RX/TX queues
	bsp_circ_init(&usart_rx, usart_rx_buf, sizeof(usart_rx_buf));
	bsp_circ_init(&usart_tx, usart_tx_buf, sizeof(usart_tx_buf));
		
	// ISR's
	INTC_register_interrupt(usart_isr_handler, MISC_USART_IRQ, AVR32_INTC_INT0);
	MISC_USART->ier = AVR32_USART_IER_RXRDY_MASK;
	
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		if (bsp_circ_size(&usart_rx) > 0)
		{
			ch = bsp_circ_readb(&usart_rx);
			//print_dbg_char(ch);
			bsp_circ_write(&usart_tx, "Hello\r\n", 7);
		}
		
		if (bsp_circ_size(&usart_tx) > 0)
		{
			MISC_USART->ier = AVR32_USART_IER_TXRDY_MASK;
		}
		else
		{
			MISC_USART->idr = AVR32_USART_IDR_TXRDY_MASK;
		}
	}
}