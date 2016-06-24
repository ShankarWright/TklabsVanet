/**
 *	@file	tkvs_timer.c
 *
 *	@brief	TKVS Timer Routines
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013-2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <asf.h>
#include "vanet.h"
#include "conf_apps.h"

#ifdef CONFIG_BSP_ENABLE_TKVS

#ifdef CONFIG_BSP_UCOS

#define CHECK_TIMER()   { if (timer->signature != TKVS_TIMER_SIGNATURE) print_dbg("TKVS Timer Error"); }
    
static void s_timer_callback(void *p_timer, void *cb_msg)
{
    INT8U perr;
    bsp_tkvs_timer_t *timer = (bsp_tkvs_timer_t *)cb_msg;
    
    if (timer->type == BSP_TKVS_TIMER_ONE_SHOT)
    {
        // one shot timer automatically freed
        timer->active = false;
        OSTmrDel(p_timer, &perr);
        timer->timer = NULL;
    }
    
    // send timer event
    bsp_tkvs_publish_immed(timer->task, timer->event, timer->id);
}

void bsp_tkvs_start_timer(bsp_tkvs_timer_t *timer)
{
    INT8U perr;
    
    CHECK_TIMER();
    
    // uC/OS - WTF
    // The timer timeout is only settable via OSTmrCreate()!
    if (timer->timer && timer->timeout != timer->previous_timeout)
    {
        // trying to re-use and existing timer but the timeout is different
        //D_DBG("TKVS Timer: Reusing timer with new timeout - doing OSTmrDel/Create");
        OSTmrDel(timer->timer, &perr);
        OS_CHECK_PERR(perr, "AMCS Start Timer: Timeout Changed");
        
        timer->timer = NULL;
    }
    
    // only create a timer if we haven't already
    if (timer->timer == NULL)
    {   
        if (timer->type == BSP_TKVS_TIMER_PERIODIC)
        {
            timer->timer = OSTmrCreate(0, timer->timeout, OS_TMR_OPT_PERIODIC, s_timer_callback, (void *)timer, (INT8U *)"", &perr);
            OS_CHECK_PERR(perr, "TKVS Start Timer: Periodic");
        }
        else
        {
            timer->timer = OSTmrCreate(timer->timeout, 0, OS_TMR_OPT_ONE_SHOT, s_timer_callback, (void*)timer, (INT8U *)"", &perr);
            OS_CHECK_PERR(perr, "TKVS Start Timer: One Shot");
        }    
    
        timer->active = true;
        timer->previous_timeout = timer->timeout;
    }    
    
    // (re)start timer
    OSTmrStart(timer->timer, &perr);
    OS_CHECK_PERR(perr, "TKVS Start Timer");
}

void bsp_tkvs_stop_timer(bsp_tkvs_timer_t *timer)
{
    INT8U perr;
    
    CHECK_TIMER();
    
    // can only stop timers that are running
    if (timer->active)
    {
        // stop the timer - don't call the callback
        timer->active = false;
        OSTmrStop(timer->timer, OS_TMR_OPT_NONE, NULL, &perr);
        
        // also delete the timer
        OSTmrDel(timer->timer, &perr);
        timer->timer = NULL;
    }
}

void bsp_tkvs_init_timer(bsp_tkvs_timer_t *timer, uint8_t type, uint8_t id, uint8_t task, uint16_t event, uint16_t timeout)
{
    // internal stuff
    timer->active = false;
    timer->signature = TKVS_TIMER_SIGNATURE;
    timer->timer = NULL;
    
    // user supplied params
    timer->type = type;
    timer->id = id;
    timer->task = task;
    timer->event = event;
    timer->timeout = timeout;
    timer->previous_timeout = -1;
}


void bsp_tkvs_set_timer(bsp_tkvs_timer_t *timer, uint16_t timeout)
{
    CHECK_TIMER();
    
    timer->timeout = timeout;
}


uint16_t bsp_tkvs_get_timer(bsp_tkvs_timer_t *timer)
{
    CHECK_TIMER();
    
    return timer->timeout;
}

#else	// CONFIG_BSP_UCOS

#error "TKVS Requires UCOS"

#endif	// CONFIG_BSP_UCOS

#endif // CONFIG_BSP_ENABLE_TKVS