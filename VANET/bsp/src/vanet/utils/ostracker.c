/**
 *	@file	ostracker.c
 *
 *	@brief	Utilities to track OS behavior
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include "asf.h"
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_OSTRACKER

#define NUM_TASK_SWITCHES           128

#if OS_DEBUG_EN > 0u
extern INT16U  const  OSDebugEn;
extern INT16U  const  OSEventEn;
extern INT16U  const  OSEventMax;
extern INT16U  const  OSFlagEn;
extern INT16U  const  OSFlagMax;
extern INT16U  const  OSQEn;
extern INT16U  const  OSQMax;
extern INT16U  const  OSTmrEn;
extern INT16U  const  OSTmrCfgMax;
extern OS_TMR  OSTmrTbl[];
#endif // OS_DEBUG_EN

typedef struct
{
    int8_t pri;
    int8_t status;
    int8_t pend_status;
    uint8_t pad;
    uint32_t uptime;
} task_switch_t;

static task_switch_t s_switches[NUM_TASK_SWITCHES];
static uint8_t s_switches_head;
static bool s_block_switch;

static void stack_handler(int argc, char** argv, uint8_t port)
{
    bsp_print_stack_usage(port);
}

static bsp_sti_command_t stack_command =
{
    .name = "stack",
    .handler = &stack_handler,
    .minArgs = 0,
    .maxArgs = 0,
    STI_HELP("stack                                 Show stack usage")
};

static void os_cfg_handler(int argc, char** argv, uint8_t port)
{
    bsp_print_os_config(port);
}

static bsp_sti_command_t os_cfg_command =
{
    .name = "os",
    .handler = &os_cfg_handler,
    .minArgs = 0,
    .maxArgs = 0,
    STI_HELP("os                                    Show OS Config")
};

static void sw_cfg_handler(int argc, char** argv, uint8_t port)
{
    bsp_print_stack_switches(port);
}

static bsp_sti_command_t sw_cfg_command =
{
    .name = "sw",
    .handler = &sw_cfg_handler,
    .minArgs = 0,
    .maxArgs = 0,
    STI_HELP("sw                                    Show OS Task Switches")
};

void bsp_ostracker_init(void)
{
    memset(s_switches,0,sizeof(s_switches));
    s_switches_head = 0;
    s_block_switch = false;
    
    bsp_sti_register_command(&stack_command);
    bsp_sti_register_command(&os_cfg_command);
    bsp_sti_register_command(&sw_cfg_command);
}

void bsp_print_os_config(uint8_t port)
{
    #if OS_DEBUG_EN > 0u
    int i;
    bsp_tkvs_timer_t *timer;
    bsp_termios_printf(port, "OSDebugEn = %d\r\n", OSDebugEn);
    bsp_termios_printf(port, "OSEventEn = %d\r\n", OSEventEn);
    bsp_termios_printf(port, "OSEventMax = %d\r\n", OSEventMax);
    bsp_termios_printf(port, "OSFlagEn = %d\r\n", OSFlagEn);
    bsp_termios_printf(port, "OSFlagMax = %d\r\n", OSFlagMax);
    bsp_termios_printf(port, "OSQEn = %d\r\n", OSQEn);
    bsp_termios_printf(port, "OSQMax = %d\r\n", OSQMax);
    bsp_termios_printf(port, "OSTmrEn = %d\r\n", OSTmrEn);
    bsp_termios_printf(port, "OSTmrCfgMax = %d\r\n", OSTmrCfgMax);
    for (i=0; i<OSTmrCfgMax; i++)
    {
        bsp_termios_printf(port, "    Timer %d, State %d\r\n", i, OSTmrTbl[i].OSTmrState);
        timer = (bsp_tkvs_timer_t *)OSTmrTbl[i].OSTmrCallbackArg;
        if (timer && timer->signature == TKVS_TIMER_SIGNATURE)
        {
            bsp_termios_printf(port, "\tAMVS: ");
            switch (timer->type)
            {
                case BSP_TKVS_TIMER_PERIODIC:
                bsp_termios_printf(port, "Periodic ");
                break;
                case BSP_TKVS_TIMER_ONE_SHOT:
                bsp_termios_printf(port, "OneShot ");
                break;
                default:
                bsp_termios_printf(port, "Unknown? ");
                break;
            }
            bsp_termios_printf(port, " id=%d task=%d event=%d timeout=%d", timer->id, timer->task, timer->event, 
                BSP_TKVS_TIMER_TICKS_TO_MS(timer->timeout));
        }
        else
        {
            bsp_termios_printf(port, "\tDly: %d Per: %d Opt: %02X CB: %08X", OSTmrTbl[i].OSTmrDly, OSTmrTbl[i].OSTmrPeriod,
                OSTmrTbl[i].OSTmrOpt, (uint32_t)OSTmrTbl[i].OSTmrCallback);
        }
        bsp_termios_printf(port, "\r\n");
    }
    #endif // OS_DEBUG_EN
}

void bsp_print_stack_usage(uint8_t port)
{
    OS_STK_DATA data;
    uint8_t i;
    INT8U perr;
    INT8U *task_name_ptr = (INT8U*) "";
    
    bsp_termios_printf(port,"            Task  Pri      Used    Free\r\n");
    for (i=0; i<OS_LOWEST_PRIO; i++)
    {
        if (OSTaskStkChk(i,&data) == OS_ERR_NONE)
        {
            if (OSRunning) OSTaskNameGet(i, &task_name_ptr, &perr);
            bsp_termios_printf(port,"%16s %4d %8d %8d\r\n", task_name_ptr, i, data.OSUsed, data.OSFree);
        }
    }
}

void App_TaskSwHook(void)
{
    if (!s_block_switch)
    {
        task_switch_t* x = &s_switches[s_switches_head];
        x->pri = OSTCBHighRdy->OSTCBPrio;
        x->status = OSTCBHighRdy->OSTCBStat;
        x->pend_status = OSTCBHighRdy->OSTCBStatPend;
        x->uptime = 0; // TBD - bsp_rtc_get_uptime();
        
        s_switches_head++;
        if (s_switches_head >= NUM_TASK_SWITCHES) s_switches_head = 0;
    }
}

void bsp_print_stack_switches(uint8_t port)
{
    INT8U perr;
    INT8U *task_name_ptr = (INT8U*) "";
    int i;
    
    s_block_switch = true;
    
    //         0123456789 0123456789012345 0123 01   01
    bsp_termios_printf(port, "      Time             Task  Pri Stat PStat\r\n");
    
    for (i=0; i<NUM_TASK_SWITCHES; i++)
    {
        task_switch_t* x = &s_switches[(s_switches_head + i) % NUM_TASK_SWITCHES];

        if (OSRunning) OSTaskNameGet(x->pri, &task_name_ptr, &perr);
        bsp_termios_printf(port, "%10d %16s %4d %02X   %02X\r\n", x->uptime, task_name_ptr, x->pri, x->status&0xff, x->pend_status&0xff);
        
        if (OSRunning && (i&0xf)==0xf) OSTimeDly(1);
    }
    
    s_block_switch = false;
}

#endif