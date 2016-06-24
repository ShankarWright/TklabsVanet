/**
 *	@file	pin.c
 *
 *	@brief	I/O Pin Driver
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
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_PIN

struct io_pin_config
{
    uint32_t    pin;
    uint16_t    event;
    uint16_t    active_time;
    uint8_t     active_level;
    uint8_t     enable_pull_up_down;
};

#define PIN_CFG_NAME_LINE(N,unused) BSP_PIN_##N##_CFG,

static struct io_pin_config s_io_pin[] = {
    MREPEAT(BSP_PIN_COUNT,PIN_CFG_NAME_LINE,~)
};

struct io_pin_state
{
    bool        changed;
    int8_t      last_state;
    uint8_t     required_count;
    uint8_t     current_count;
};

static struct io_pin_state s_io_pin_states[BSP_PIN_COUNT];
    
static void pin_int_handler(void)
{    
    // check for each pin and send event if changed
    for (int i=0; i<BSP_PIN_COUNT; i++)
    {
        // clear all my button interrupts
        if (gpio_get_pin_interrupt_flag(s_io_pin[i].pin))
        {
            gpio_clear_pin_interrupt_flag(s_io_pin[i].pin);
            
            // reset our pin state
            s_io_pin_states[i].changed = true;
            s_io_pin_states[i].current_count = 0;
        }
    }
}

void bsp_pin_check(void)
{
    bool active;
    
    for (int i=0; i<BSP_PIN_COUNT; i++)
    {
        if (s_io_pin_states[i].changed)
        {
            ++s_io_pin_states[i].current_count;
            if (s_io_pin_states[i].current_count > s_io_pin_states[i].required_count)
            {
                s_io_pin_states[i].changed = false;
                
                // publish!
                if (gpio_get_pin_value(s_io_pin[i].pin) == s_io_pin[i].active_level)
                    active = true;
                else
                    active = false;
                    
                //DBG if (s_io_pin_states[i].last_state != active)
                {
                    bsp_logcat_printf(BSP_LOGCAT_DEBUG, "PIN: %04x = %d", s_io_pin[i].event, active);
                    bsp_tkvs_publish_immed(BSP_TKVS_SRC_PIN, s_io_pin[i].event, active);
                    s_io_pin_states[i].last_state = active;
                }
                //DBG else
                //DBG {
                //DBG     bsp_logcat_printf(BSP_LOGCAT_DEBUG, "PIN: %04x Glitched - Ignoring", s_io_pin[i].event);
                //DBG }
            }
        }
    }
}

void bsp_pin_init(void)
{
    // initialize all pins used as buttons and attach the keypad ISR to them
    for (int i=0; i<BSP_PIN_COUNT; i++)
    {
        INTC_register_GPIO_interrupt(&pin_int_handler, s_io_pin[i].pin);
        gpio_enable_gpio_pin(s_io_pin[i].pin);
        gpio_enable_pin_glitch_filter(s_io_pin[i].pin);
        if (s_io_pin[i].enable_pull_up_down)
        {
            if (s_io_pin[i].active_level == 1)
                gpio_enable_pin_pull_down(s_io_pin[i].pin);
            else
                gpio_enable_pin_pull_up(s_io_pin[i].pin);
        }
        gpio_enable_pin_interrupt(s_io_pin[i].pin, GPIO_PIN_CHANGE);
        
        // set pin state
        s_io_pin_states[i].changed = true;      // force the initial state to be published
        s_io_pin_states[i].current_count = 0;
        s_io_pin_states[i].last_state = -1;     // impossible last state
        // convert ms to ticks
        s_io_pin_states[i].required_count = s_io_pin[i].active_time * CONFIG_BSP_RTC_TICK_HZ / 1000;
    }
	
	// register my idle function
	bsp_idle_register_idle_function(bsp_pin_check, BSP_IDLE_ONCE_PER_TICK);
}


void bsp_pin_read(enum bsp_pin_events event)
{
    // for the pin requested send event with current state
    for (int i=0; i<BSP_PIN_COUNT; i++)
    {
        // changed == false makes us only report the debounced state!
        if (event == s_io_pin[i].event && s_io_pin_states[i].changed == false)
        {
            if (gpio_get_pin_value(s_io_pin[i].pin) == s_io_pin[i].active_level)
                bsp_tkvs_publish_immed(BSP_TKVS_SRC_PIN, s_io_pin[i].event, 1);
            else
                bsp_tkvs_publish_immed(BSP_TKVS_SRC_PIN, s_io_pin[i].event, 0);
        }
    }
}

#endif // CONFIG_BSP_ENABLE_PIN