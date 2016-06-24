/**
 *	@file	termios.c
 *
 *	@brief	General terminal interface to control asynchronous communications ports combined with the low level driver.
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

#ifdef CONFIG_BSP_ENABLE_TERMIOS

/// BSP_TKVS_SRC_TERMIOS_INTERNAL Events
enum
{
	BSP_TERMIOS_INTERNAL_RX		= 0x0001,
	BSP_TERMIOS_INTERNAL_TX		= 0x0002,
};

struct termios_config
{
	const char				*name;
	volatile avr32_usart_t	*usart;
	uint32_t				initial_baud;
	uint32_t				irq;
	uint16_t				rx_buf_size;
	uint16_t				tx_buf_size;
	uint8_t					mode;
	uint8_t					echo;
	uint8_t					buffer;
	uint8_t					rx_pin;
	uint8_t					rx_function;
	uint8_t					tx_pin;
	uint8_t					tx_function;
};

struct termios_state
{
	bsp_termios_mode		mode;			// canonical, raw, ???
	bsp_termios_buffer		buffer;			// none or ???	
	bool					echo;			// echo
	bool					escapeNext;		// handle VT100 escape sequences
	uint8_t					cmdLen;			// length of current command
	char					cmd[128];		// command
	bsp_termios_idle_handler_t handler;		// idle handler
};

// Termios Port Configuration
#define TERMIOS_CFG_NAME_LINE(N,unused) BSP_TERMIOS_##N##_CFG,
static const struct termios_config s_termios[] = {
	MREPEAT(BSP_TERMIOS_COUNT,TERMIOS_CFG_NAME_LINE,~)
};
static struct termios_state s_termios_state[BSP_TERMIOS_COUNT];

// Termios Buffers
static bsp_circ_buffer_t s_termios_rx[BSP_TERMIOS_COUNT];
static bsp_circ_buffer_t s_termios_tx[BSP_TERMIOS_COUNT];

// PTI-Lite Termios
uint8_t s_termios_lite;

// Termios Interrupt Handlers
static void bsp_termios_port_isr(uint8_t port)
{
	int c; 
	if (s_termios[port].usart->csr & AVR32_USART_CSR_RXRDY_MASK) 
	{ 
		if (usart_read_char(s_termios[port].usart, &c) == USART_RX_ERROR) 
		{
			usart_reset_status(s_termios[port].usart); 
			return; 
		} 
		else 
		{ 
			bsp_circ_writeb(&s_termios_rx[port], c); 
		} 
	} 
	if (s_termios[port].usart->csr & AVR32_USART_CSR_TXRDY_MASK) 
	{ 
		c = bsp_circ_readb(&s_termios_tx[port]); 
		if (c >= 0) 
		{ 
			s_termios[port].usart->thr = (c << AVR32_USART_THR_TXCHR_OFFSET) & AVR32_USART_THR_TXCHR_MASK; 
		} 
		else 
		{ 
			s_termios[port].usart->idr = AVR32_USART_IDR_TXRDY_MASK; 
		} 
	} 
}

#define BSP_TERMIOS_INTERRUPT_CODE(N, unused) BSP_INT_ATTR static void bsp_termios_port_##N##_isr(void) \
{ \
	bsp_termios_port_isr(N); \
}
MREPEAT(BSP_TERMIOS_COUNT, BSP_TERMIOS_INTERRUPT_CODE, ~)

// Termios Pointers to Interrupt Handlers
#define BSP_TERMIOS_INTERRUPT_PTRS(N, unused) &bsp_termios_port_##N##_isr,
static const void *s_termios_isr_ptrs[] = {
	MREPEAT(BSP_TERMIOS_COUNT,BSP_TERMIOS_INTERRUPT_PTRS,~)
};	


static OS_STK s_termios_task_stack[TASK_TERMIOS_STACK_SIZE];
static void bsp_termios_task(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[32];

static void bsp_termios_enable_tx(uint8_t port)
{
	if (bsp_circ_size(&s_termios_tx[port]) >= 0)
	{
		s_termios[port].usart->ier = AVR32_USART_IER_TXRDY_MASK;
	}
	else
	{
		s_termios[port].usart->idr = AVR32_USART_IDR_TXRDY_MASK;
	}
}

void bsp_termios_init(void)
{	
	void *buf;
	usart_options_t generic_usart_options =
	{
		.baudrate = 9600,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};
    
	// initialize each port
	for (int i=0; i<BSP_TERMIOS_COUNT; i++)
	{
		// initial port buffers
		buf = bsp_malloc(s_termios[i].rx_buf_size);
		bsp_circ_init(&s_termios_rx[i], buf, s_termios[i].rx_buf_size);
		buf = bsp_malloc(s_termios[i].tx_buf_size);
		bsp_circ_init(&s_termios_tx[i], buf, s_termios[i].tx_buf_size);

		generic_usart_options.baudrate = s_termios[i].initial_baud;
		
        if ((s_termios[i].usart->mr & 0x0000ffff) == 0)	// the main usart_mode pins
        {
			//print_dbg("Initialazing USART "); print_dbg_int(i); print_dbg("\r\n");
			
            // set the gpio pins to their needed functions
            gpio_enable_module_pin(s_termios[i].rx_pin, s_termios[i].rx_function);
            gpio_enable_module_pin(s_termios[i].tx_pin, s_termios[i].tx_function);

		    // initial the port itself
		    usart_init_rs232(s_termios[i].usart, &generic_usart_options, sysclk_get_peripheral_bus_hz((void *)s_termios[i].usart));
        }            
		
		// register interrupt handler
		INTC_register_interrupt(s_termios_isr_ptrs[i], s_termios[i].irq, AVR32_INTC_INT0);

		// enable receive interrupts
		s_termios[i].usart->ier = AVR32_USART_IER_RXRDY_MASK;
		
		// Assume Canonical Mode & Echo
		s_termios_state[i].mode = s_termios[i].mode;
		s_termios_state[i].echo = s_termios[i].echo;
		s_termios_state[i].buffer = s_termios[i].buffer;
		s_termios_state[i].escapeNext = false;
		s_termios_state[i].cmdLen = 0;
		s_termios_state[i].cmd[0] = '\0';
		s_termios_state[i].handler = NULL;
	}
	
	// Create our Task
	INT8U perr;
	
	// debug console message
	print_dbg("Creating Termios Task\r\n");
	
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "termios task: OSQCreate");
	
	// Create the hw task
	perr = OSTaskCreateExt(bsp_termios_task,
		(void *)0,
		(OS_STK *)&s_termios_task_stack[TASK_TERMIOS_STACK_SIZE - 1],
		TASK_TERMIOS_PRIO,
		TASK_TERMIOS_PRIO,
		(OS_STK *)&s_termios_task_stack[0],
		TASK_TERMIOS_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OS_CHECK_PERR(perr, "termios task: OSTaskCreateExt");
	
	OSTaskNameSet(TASK_TERMIOS_PRIO, (INT8U *)"Termios Task", &perr);
	OS_CHECK_PERR(perr, "termios task: OSTaskNameSet");
	
	bsp_tkvs_subscribe(BSP_TKVS_SRC_TERMIOS_INTERNAL, BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_TERMIOS_PRIO);
	
	if (bsp_termios_find_port("Extra", &s_termios_lite))
	{
		bsp_tkvs_subscribe(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_lite), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_TERMIOS_PRIO);
	}
	
	bsp_idle_register_idle_function(bsp_termios_idle_loop, BSP_IDLE_ALWAYS);
}

void bsp_pti_lite(bsp_tkvs_msg_t *msg);

void bsp_termios_task(void* p_arg)
{
	(void) p_arg;
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	int c;
	uint8_t port;
	
	// inform users the port is open for business!	
	for (int i=0; i<BSP_TERMIOS_COUNT; i++)
	{
		/*
		print_dbg("Termios: Starting ");
		print_dbg(s_termios[i].name);
		print_dbg(" on Port ");
		print_dbg_int(i);
		print_dbg_char(' ');
		print_dbg_int(s_termios_state[i].mode);
		print_dbg_char(' ');
		print_dbg_int(s_termios_state[i].echo);
		print_dbg_char(' ');
		print_dbg_int(s_termios_state[i].buffer);
		print_dbg("\r\n");
		*/
		bsp_tkvs_publish_immed(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(i), BSP_TERMIOS_INPUT_READY, 0);
	}
		
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);
		OS_CHECK_PERR(perr, "termios task: OSQPend");
		if (msg != (void *)0)
		{
			if (msg->source == BSP_TKVS_SRC_TERMIOS_INTERNAL)
			{
				if (msg->event == BSP_TERMIOS_INTERNAL_RX)
				{
					// parse input data
					port = msg->immed_data;
					//print_dbg_char('[');print_dbg_char('0'+port);print_dbg_char(']');
					
					if (s_termios_state[port].mode == BSP_TERMIOS_MODE_CANONICAL)
					{
						//print_dbg_char('C');
						// In Canonical Mode - Byte at a time!
						while ((c = bsp_circ_readb(&s_termios_rx[port])) > 0)
						{
							if (c == 0x1b)	// VT100 processing
							{
								s_termios_state[port].escapeNext = true;
							}
							else if (s_termios_state[port].escapeNext)	// VT100 processing
							{
								if (c == 'A')
								{
									bsp_tkvs_publish_immed(BSP_TKVS_SRC_TERMIOS_PORT_START + port,
										BSP_TERMIOS_INPUT_SIGNAL, BSP_TERMIOS_SIGNAL_UP);
								}
								else if (c == 'B')
								{
									bsp_tkvs_publish_immed(BSP_TKVS_SRC_TERMIOS_PORT_START + port,
										BSP_TERMIOS_INPUT_SIGNAL, BSP_TERMIOS_SIGNAL_DOWN);
								}
								
								if (c != '[')
								{
									s_termios_state[port].escapeNext = false;
								}
							}
							else if (c == '\b')
							{
								if (s_termios_state[port].cmdLen > 0)
								{
									s_termios_state[port].cmdLen--;
									if (s_termios_state[port].echo)
									{
										bsp_circ_writeb(&s_termios_tx[port], '\b');
										bsp_circ_writeb(&s_termios_tx[port], ' ');
										bsp_circ_writeb(&s_termios_tx[port], '\b');
									}
								}
							}
							else if (c == 0x03)
							{
								// abort command and signal
								s_termios_state[port].cmdLen = 0;
								bsp_tkvs_publish_immed(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(port), 
											BSP_TERMIOS_INPUT_SIGNAL, BSP_TERMIOS_SIGNAL_INTERRUPT);
							}
							else if (c == '\r')
							{
								// completed command
								s_termios_state[port].cmd[s_termios_state[port].cmdLen] = '\0';
								
								//print_dbg(s_termios_state[port].cmd); print_dbg("\r\n");
								
								bsp_tkvs_publish_data(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(port),\
											BSP_TERMIOS_INPUT_RX, s_termios_state[port].cmd, s_termios_state[port].cmdLen+1);
								
								// get ready for next line
								s_termios_state[port].cmdLen = 0;
								s_termios_state[port].cmd[0] = '\0';
								
							}
							else if (c == '\n')
							{
								// just ignore these characters in canonical mode
							}
							else
							{
								s_termios_state[port].cmd[s_termios_state[port].cmdLen++] = c;
								
								if (s_termios_state[port].echo)
								{
									bsp_circ_writeb(&s_termios_tx[port], c);
								}									
							}
							
						
							// enable tx if data
							bsp_termios_enable_tx(port);
						}														
					}
					else if (s_termios_state[port].mode == BSP_TERMIOS_MODE_RAW)
					{
						// raw mode - currently we ignore 'echo' flag in raw mode
						//print_dbg("Got data on port "); print_dbg_int(port); print_dbg("\r\n");
						int num_bytes = bsp_circ_size(&s_termios_rx[port]);
						if (num_bytes > 0)
						{
							bsp_tkvs_msg_t* msg = bsp_tkvs_alloc(num_bytes);
							bsp_circ_read(&s_termios_rx[port], msg->data, num_bytes);
							msg->data_len = num_bytes;
							//print_dbg_char('R');
							bsp_tkvs_publish(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(port), BSP_TERMIOS_INPUT_RX, msg);
						}
					}
					else
					{
						print_dbg("Unknown Mode on Port "); 
						print_dbg_int(port); print_dbg_char(' '); 
						print_dbg_int(s_termios_state[port].mode);
						print_dbg("\r\n");
					}
					
				}
				else if (msg->event == BSP_TERMIOS_INTERNAL_TX)
				{
					bsp_termios_enable_tx(msg->immed_data);
				}
			}
			else if (msg->source == BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_lite))
			{
				bsp_pti_lite(msg);
			}
			bsp_tkvs_free(msg);
		}			
	}
}	

// This will run on essentially every byte received unless we sleep in the idle loop
// This code throttles the serial so it isn't called more than once per tick...
// If we *are* calling sleep in the idle loop - this should do nothing!	
static INT32U s_last_tick = 0;
static INT32U s_idle_task_tick = 0;
#define BSP_TERMIOS_BUFFER_MIN_TICKS	6
void bsp_termios_idle_loop(void)
{
	uint16_t size, watermark;
	INT32U current_tick = OSTimeGet();
	bool bufferTimeout = false;
	
	if ((current_tick - s_last_tick) > BSP_TERMIOS_BUFFER_MIN_TICKS)
	{
		bufferTimeout = true;
		s_last_tick = current_tick;
	}
	
	for (int i=0; i<BSP_TERMIOS_COUNT; i++)
	{
		size = bsp_circ_size(&s_termios_rx[i]);
		watermark = bsp_circ_length(&s_termios_rx[i]) >> 1;
		if (size > 0)
		{
			if (s_termios_state[i].buffer == BSP_TERMIOS_BUFFER_NONE || bufferTimeout || size > watermark)
			{
				/*
				print_dbg_char('0' + i);
				if (s_termios_state[i].buffer == BSP_TERMIOS_BUFFER_NONE)
					print_dbg_char('N');
				else if (bufferTimeout)
					print_dbg_char('T');
				else 
					print_dbg_char('W');
				*/
				
				// inform data on buffer - future we might send a pointer to the buffer instead					
				bsp_tkvs_publish_immed(BSP_TKVS_SRC_TERMIOS_INTERNAL, BSP_TERMIOS_INTERNAL_RX, i);
			}
		}
	}
	
	// only call idle task handlers once per tick
	if (current_tick > s_idle_task_tick)
	{
		
		for (int i=0; i<BSP_TERMIOS_COUNT; i++)
		{
			if (s_termios_state[i].handler != NULL)
			{
				s_termios_state[i].handler(i);
			}
		}
		s_idle_task_tick = current_tick;
	}	
}

/*
 * Termios Public API 
 */

#ifdef CONFIG_BSP_ENABLE_STI
/* -----------
 *  TERMIOS STI
 * ----------*/
static void termios_handler(int argc, char** argv, uint8_t port)
{
	bsp_termios_write_str(port, "Termios State\r\n");
	for (int i=0; i<BSP_TERMIOS_COUNT; i++)
	{
		bsp_termios_printf(port, "Termio %d (%s) Mode=%d, Echo=%d, Buffer=%d\r\n", i, s_termios[i].name,
			s_termios_state[i].mode, s_termios_state[i].echo, s_termios_state[i].buffer);
	}	
}

bsp_sti_command_t termios_command =
{
	.name = "termios",
	.handler = &termios_handler,
	.minArgs = 0,
	.maxArgs = 0
};
#endif // CONFIG_BSP_ENABLE_STI

bool bsp_termios_find_port(const char *name, uint8_t *port)
{
	for (int i=0; i<BSP_TERMIOS_COUNT; i++)
	{
		if (strcmp(s_termios[i].name, name) == 0)
		{
			*port = i;
			return true;
		}
	}
	return false;
}

void bsp_termios_register_idle_handler(uint8_t port, bsp_termios_idle_handler_t handler)
{
	s_termios_state[port].handler = handler;
}

void bsp_termios_set_buffer(uint8_t port, bsp_termios_buffer buffer)
{
	s_termios_state[port].buffer = buffer;
}

int bsp_termios_printf(uint8_t port, const char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
    char* buf = (char*) bsp_malloc(256);
	int len = dlib_vsnprintf(buf,256,fmt,va);
	if (len > 0)
	{
		bsp_termios_write(port, (const unsigned char *)buf, len);
	}
    bsp_free(buf);
	return len;
}

void bsp_termios_write(uint8_t port, const unsigned char *buf, uint16_t len)
{
    if (port == BSP_TERMIOS_RAW_DEBUG_PORT)
    {
        while (len-- > 0)
        {
            usart_putchar(DBG_USART, *buf++);
        }
    }
    else
    {        
	    uint16_t qlen = bsp_circ_length(&s_termios_tx[port]);
	    uint16_t remaining = len;
	    uint16_t needed;
	    uint16_t wpos = 0;
	
	    while (remaining > 0)
	    {
		    needed = min(qlen, remaining);
		
		    // wait until we have this amount of space
		    bsp_termios_drain(port, needed);
		
		    // queue it!
		    bsp_circ_write(&s_termios_tx[port], &buf[wpos], needed);
		
		    remaining -= needed;
		    wpos += needed;
        }
    }    	    
}

void bsp_termios_flush(uint8_t port)
{
	// called from our task - do it direct, otherwise send a message
	if (OSPrioCur == TASK_TERMIOS_PRIO)
		bsp_termios_enable_tx(port);
	else
		bsp_tkvs_publish_immed(BSP_TKVS_SRC_TERMIOS_INTERNAL, BSP_TERMIOS_INTERNAL_TX, port);
}

void bsp_termios_drain(uint8_t port, uint16_t needed)
{
	uint16_t free;
	
	if (needed == UINT16_MAX)
		needed = bsp_circ_length(&s_termios_tx[port]) - 1;		// i.e. max free space
		
	free = bsp_circ_free(&s_termios_tx[port]);
	
	if (free < needed)
	{
		// first make sure we're sending
		bsp_termios_flush(port);
		
		// wait until we have enough space
		while (free < needed)
		{
			bsp_delay(50);
			free = bsp_circ_free(&s_termios_tx[port]);
		}
	}
}

bool bsp_termios_set_input(uint8_t port, const char *string)
{
	if (s_termios_state[port].mode == BSP_TERMIOS_MODE_CANONICAL)
	{
		uint16_t len = strlen(string);
		
		s_termios_state[port].cmdLen = len;
		strcpy(s_termios_state[port].cmd, string);
		
		bsp_termios_write_str(port, string);
		bsp_termios_flush(port);
		return true;
	}
	else
	{
		// only allowed in Canonical Mode
		return false;
	}
}

#define PTI_LITE_PROMPT "\r\nPROMPT> "
void bsp_pti_lite(bsp_tkvs_msg_t *msg)
{
	if (msg->event == BSP_TERMIOS_INPUT_RX)
	{
		// INPUT_RX is a C-String (terminated with '\0')
		if (msg->data[0] != '\0')
		{
			if (strcasecmp((const char *)msg->data, "reset") == 0)
			{
				bsp_termios_write_str(s_termios_lite, "Resetting board...\r\n");
				bsp_termios_enable_tx(s_termios_lite);
				reset_do_soft_reset();
			}
			else
			{
				bsp_termios_write_str(s_termios_lite, "\r\nUnknown Command: ");
				bsp_termios_write_str(s_termios_lite, (const char *)msg->data);
				bsp_termios_enable_tx(s_termios_lite);
			}
		}		
		bsp_termios_write_str(s_termios_lite, PTI_LITE_PROMPT);
	}
	else if (msg->event == BSP_TERMIOS_INPUT_SIGNAL)
	{
		if (msg->immed_data == BSP_TERMIOS_SIGNAL_INTERRUPT)
		{
			bsp_circ_write(&s_termios_tx[s_termios_lite], "^C\r\nPROMPT> ", 12);
			bsp_termios_enable_tx(s_termios_lite);
		}
	}
	else if (msg->event == BSP_TERMIOS_INPUT_READY)
	{
		bsp_termios_write_str(s_termios_lite, "\r\n\r\n");
		bsp_termios_write_str(s_termios_lite, "PTI Lite\r\n\r\n");
		bsp_termios_write_str(s_termios_lite, PTI_LITE_PROMPT);
		bsp_termios_enable_tx(s_termios_lite);
	}
}

#endif // CONFIG_BSP_ENABLE_TERMIOS