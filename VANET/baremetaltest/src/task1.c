/**
 *	@file	task1.c
 *
 *	@brief	Basic uC/OS Task for Testing
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

#ifdef CONFIG_BSP_UCOS

static OS_STK s_task1_stack[TASK1_STACK_SIZE];
static void app_task1(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[16];

extern void app_task1_init(void);

/* 
 * Timer Callback
 */
static void app_task1_cb(void *p_timer, void *cb_msg)
{
	gpio_toggle_pin(AVR32_PIN_PC11);
}

/*
 * Initialize Hardware Task
 */
void app_task1_init()
{
    INT8U perr;
    
    // debug console message
    print_dbg("Creating Task 1\r\n");
    gpio_configure_pin(AVR32_PIN_PC11, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
	
    // create a message queue
    s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
    OS_CHECK_NULL(s_msg_flag, "task1: OSQCreate");
    
    // Create the hw task
    perr = OSTaskCreateExt(app_task1,
        (void *)0,
        (OS_STK *)&s_task1_stack[TASK1_STACK_SIZE - 1],
        TASK1_PRIO,
        TASK1_PRIO,
        (OS_STK *)&s_task1_stack[0],
        TASK1_STACK_SIZE,
        (void *)0,
        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OS_CHECK_PERR(perr, "task1: OSTaskCreateExt");
    
    OSTaskNameSet(TASK1_PRIO, (INT8U *)"Task 1", &perr);
    OS_CHECK_PERR(perr, "task1: OSTaskNameSet");
}

/*
 * Task 1
 */
static void app_task1(void* p_arg)
{
    void *msg;
    INT8U perr;
	OS_TMR *tmr;
    
    (void) p_arg;
    
	tmr = OSTmrCreate(0,
	    BSP_TKVS_TIMER_MS_TO_TICKS(30000),		// run every 30 seconds
	    OS_TMR_OPT_PERIODIC,
	    app_task1_cb,
	    (void *)0,
	    (INT8U *)"Task 1 Timer",
	    &perr);
	OS_CHECK_PERR(perr, "task1: OSTmrCreate");
	OSTmrStart(tmr, &perr);
	OS_CHECK_PERR(perr, "task1: OSTmrStart");
		
    while (1)
    {
        msg = OSQPend(s_msg_flag, 0, &perr);    // block forever on my queue - the led timer is running via OSTmr
        OS_CHECK_PERR(perr, "task1: OSQPend");
        if (msg != (void *)0)
        {
			print_dbg("Task 1 Message\r\n");
        }
    }
}    

#endif // CONFIG_BSP_UCOS