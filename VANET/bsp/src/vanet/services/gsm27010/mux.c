/**
 *	@file	mux.c
 *
 *	@brief	GSM 27.010 Basic Frame Multiplexer
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

#include <string.h>
#include <asf.h>
#include "vanet.h"
#include "conf_apps.h"
#include "mux_p.h"

#ifdef CONFIG_BSP_ENABLE_MUX

#ifndef CONFIG_BSP_UCOS
#error "MUX requires uC/OS"
#endif

uint8_t s_termios_mux;
mux_frame_t partial_frame;
mux_stat_t mux_stat[16];

static bsp_circ_buffer_t mux_rx;
static uint8_t mux_rx_buf[512];

static OS_STK s_mux_task_stack[TASK_MUX_STACK_SIZE];
static void app_mux_task(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[32];

#ifdef CONFIG_STI_CMD_MUX
/* -----------
 *  MUX
 * ----------*/
static void mux_handler(int argc, char** argv, uint8_t port)
{
	char *opened;
	
	if (argc == 3)
	{
		int dlci = atoi(argv[1]);
		int state = atoi(argv[2]);
		
		if (dlci >= BSP_TKVS_SRC_MUX_DLCI_NUM || state > 1)
		{
			bsp_termios_printf(port, "Invalid DLCI or State Provided...\r\n");
		}
		else
		{
			mux_stat[dlci].opened = state;
		}
	}
	bsp_termios_write_str(port, "MUX State\r\n");
	for (int i=0; i<BSP_TKVS_SRC_MUX_DLCI_NUM; i++)
	{
		if (mux_stat[i].opened == MUX_OPENED)
			opened = "Opened";
		else if (mux_stat[i].opened == MUX_OPEN_PENDING)
			opened = "Open Pending";
		else if (mux_stat[i].opened == MUX_CLOSE_PENDING)
			opened = "Close Pending";
		else
			opened = "Closed";
			
		bsp_termios_printf(port, "DLCI %d : %s %s\r\n", i,
			bsp_tkvs_is_subscribed(BSP_MUX_DLCI_TO_TKVS_SOURCE(i), BSP_TKVS_ALL_EVENTS) ? "Subscribed" : "Not Subscribed",
				opened);
	}
}

static bsp_sti_command_t mux_command =
{
	.name = "mux",
	.handler = &mux_handler,
	.minArgs = 0,
	.maxArgs = 2,
	STI_HELP("mux                                  Show state of MUX\r\n"
	         "mux dlci state                       Set state of DCLI, 0=closed, 1=open\r\n"
			 "  e.g. \"mux 0 0\" will Close DLCI 0")
};
#endif // CONFIG_STI_CMD_MUX

void bsp_mux_init(void)
{
	// initialize buffers
	bsp_circ_init(&mux_rx, mux_rx_buf, sizeof(mux_rx_buf));
	
	// initialize internal state
	memset(&partial_frame, 0, sizeof(partial_frame));
	for (int i=0; i<16; i++)
	{
		mux_stat[i].opened = MUX_CLOSED;
	}		
	
	// setup mux task
	INT8U perr;
	    
	// debug console message
	print_dbg("Creating Mux Task\r\n");
	    
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "mux task: OSQCreate");
	    
	// Create the hw task
	perr = OSTaskCreateExt(app_mux_task,
		(void *)0,
		(OS_STK *)&s_mux_task_stack[TASK_MUX_STACK_SIZE - 1],
		TASK_MUX_PRIO,
		TASK_MUX_PRIO,
		(OS_STK *)&s_mux_task_stack[0],
		TASK_MUX_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OS_CHECK_PERR(perr, "mux task: OSTaskCreateExt");
	    
	OSTaskNameSet(TASK_MUX_PRIO, (INT8U *)"Mux Task", &perr);
	OS_CHECK_PERR(perr, "mux task: OSTaskNameSet");
	
	// Subscribe to sources
	bsp_tkvs_subscribe(BSP_TKVS_SRC_MUX_INTERNAL, BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_MUX_PRIO);
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(0), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_MUX_PRIO);		// DLCI 0
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(BSP_TKVS_MUX_ECHO_DLCI), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_MUX_PRIO);
	
	if (bsp_termios_find_port("MUX", &s_termios_mux))
	{
		bsp_tkvs_subscribe(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_mux), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_MUX_PRIO);
	}
	else
	{
		print_dbg("Unable to find Termios Port - MUX will not work!\r\n");
	}
	
#ifdef CONFIG_STI_CMD_MUX
	bsp_sti_register_command(&mux_command);
#endif
}

void app_mux_task(void* p_arg)
{
	(void) p_arg;
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);    // block forever on my queue - the led timer is running via OSTmr
		OS_CHECK_PERR(perr, "mux task: OSQPend");
		if (msg != (void *)0)
		{
			if (msg->source == BSP_TKVS_SRC_MUX_INTERNAL)
			{
				if (msg->event ==  BSP_MUX_EVENT_DLCI_SEND)
				{
					// To-do - handle writes > 64 bytes!
					mux_write_frame(msg->immed_data, FRAME_UIH, msg->data, msg->data_len);
				}
			}
			else if (msg->source == BSP_MUX_DLCI_TO_TKVS_SOURCE(0))
			{
				bsp_logcat_printf(BSP_LOGCAT_MUX, "DLCI 0 Event: %02x", msg->event);
			}
			else if (msg->source == BSP_MUX_DLCI_TO_TKVS_SOURCE(BSP_TKVS_MUX_ECHO_DLCI))
			{
				// Echo!
				bsp_logcat_printf(BSP_LOGCAT_MUX, "DLCI ECHO Event: %02x", msg->event);
				if (msg->event == BSP_MUX_EVENT_DATA_RCVD)
				{
					if (BSP_TKVS_MSG_HAS_DATA(msg))
					{
						//print_dbg_array(msg->data, msg->data_len);
						bsp_mux_send(BSP_TKVS_SOURCE_TO_MUX_DLCI(msg->source), msg->data, msg->data_len);
					}
				}
			}
			else if (msg->source == BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_mux))
			{
				// RX data from UART via Termios
				//print_dbg_char('M');
				bsp_circ_write(&mux_rx, msg->data, msg->data_len);
				mux_recv_data(&mux_rx);
			}
			
			bsp_tkvs_free(msg);
		}
	}
}

void bsp_mux_send(uint8_t dlci, const void* data, uint16_t data_length)
{
	if (dlci < BSP_TKVS_SRC_MUX_DLCI_NUM &&
		bsp_tkvs_is_subscribed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_TKVS_ALL_EVENTS) &&
		mux_stat[dlci].opened == MUX_OPENED)
	{
		bsp_tkvs_publish_immed_with_data(BSP_TKVS_SRC_MUX_INTERNAL, BSP_MUX_EVENT_DLCI_SEND, dlci, data, data_length);
	}
}

#endif // CONFIG_BSP_ENABLE_MUX