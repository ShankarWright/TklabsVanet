/**
 *	@file	rtc.c
 *
 *	@brief	RTC - Real-time Clock Driver
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
#include "vanet.h"
#ifdef CONFIG_BSP_UCOS
#include "ucos_ii.h"
#endif

#ifdef CONFIG_BSP_ENABLE_RTC

// The AST prescaler
#define AST_PSEL                    (ilog2(CONFIG_BSP_RTC_CLOCK_HZ / CONFIG_BSP_RTC_TICK_HZ) - 1)

#define US_PER_TICK                 (1000000 / CONFIG_BSP_RTC_TICK_HZ)

static uint32_t s_uptime;           // the amount of time we've been up, in seconds
static uint32_t s_up_ticks;         // the number of ticks we've been up
static uint32_t s_tick_cycle;       // the intra-tick cycle counter
static uint32_t s_clock;            // the unix clock time
static uint32_t s_clock_us;         // the microsecond part of the uptime and clock time
static uint32_t s_tick_count;       // how many ticks into the current second we are
#ifdef CONFIG_BSP_RTC_PSEUDO_DOG_US
static uint32_t s_rtc_dog;          // the rtc watchdog ticker
#endif

#ifdef CONFIG_STI_CMD_CLOCK

static void clock_handler(int argc, char** argv, uint8_t port)
{
    calendar_date_t cal;
    uint32_t clock_ms;
    
    if (argc > 6)
    {
        // set the clock
        bsp_termios_write_str(port, "Setting Clock...\r\n");
        cal.year = strtoul(argv[1],0,10);
        cal.month = strtoul(argv[2],0,10) - 1; // january is 0
        cal.date = strtoul(argv[3],0,10) - 1; // first day of month is 0
        cal.hour = strtoul(argv[4],0,10);
        cal.minute = strtoul(argv[5],0,10);
        cal.second = strtoul(argv[6],0,10);
        bsp_rtc_set_calendar(&cal);
        bsp_tkvs_publish_immed(BSP_TKVS_SRC_CLOCK, BSP_CLOCK_EVENT_UPDATE, 0);
    }

    // show the clock
    bsp_rtc_get_calendar(&cal);
    clock_ms = bsp_rtc_get_clock_us() / 1000;
    bsp_termios_printf(port, "Clock: %u-%02d-%02d %d:%02d:%02d.%03d\r\n", 
        cal.year, cal.month+1, cal.date+1, cal.hour, cal.minute, cal.second, clock_ms);
    bsp_termios_printf(port, "Uptime: %d:%02d:%02d.%03d\r\n", 
        s_uptime / (60 * 60), (s_uptime % (60 * 60)) / 60, s_uptime % 60, clock_ms);
}

static bsp_sti_command_t clock_command =
{
    .name = "clock",
    .handler = &clock_handler,
    .minArgs = 0,
    .maxArgs = 6,
    STI_HELP("clock [<time>]                        Show and optionally set the clock\r\n"
             "\r\n"
             "Options:\r\n"
             "    <time>                            Time to set: YYYY MM dd hh mm ss\r\n"
             "\r\n"
             "Examples:\r\n"
             "    clock 2010 10 12 8 30 01          Set clock to 10/12/2010 8:30:01 AM\r\n"
             "    clock 2012 6 1 13 33 10           Set clock to 6/1/2012 1:33:10 PM")

};
#endif // CONFIG_STI_CMD_CLOCK

#ifdef CONFIG_BSP_UCOS
static void ast_int_handler(void)
#else
__attribute__((__interrupt__))
static void ast_int_handler(void)
#endif
{
    // capture the cycle count at the beginning of the tick
    s_tick_cycle = Get_sys_count();

    // clear the interrupt.  don't call ast_clear_periodic_status_flag because it waits for the value to actually
    // latch in, which takes about 90us.
    //ast_clear_periodic_status_flag(&AVR32_AST,0);
    while (AVR32_AST.sr & AVR32_AST_SR_BUSY_MASK) {}        // should always be not busy
    AVR32_AST.scr = AVR32_AST_SCR_PER0_MASK;
        
    // update the clock
    s_clock_us += US_PER_TICK;    
    if (++s_tick_count >= CONFIG_BSP_RTC_TICK_HZ)
    {
        gpio_toggle_pin(BSP_RTC_LED);
        s_clock++;
        s_uptime++;
        s_clock_us = 0;
        s_tick_count = 0;
    }
    s_up_ticks++;
    
    #ifdef CONFIG_BSP_RTC_PSEUDO_DOG_US
    if (s_rtc_dog < US_PER_TICK)
    {
        bsp_reset(BSP_RESET_RTC_DOG);
    }
    s_rtc_dog -= US_PER_TICK;
    #endif
    
    #ifdef CONFIG_BSP_UCOS
    // call the system tick
    OSTimeTick();
    #endif
}

void bsp_rtc_init(void)
{
    // we're in 1970 baby
    s_uptime = 0;
    s_tick_cycle = 0;
    s_clock = 0;
    s_clock_us = 0;
    s_tick_count = 0;
    s_up_ticks = 0;

    #ifdef CONFIG_BSP_RTC_PSEUDO_DOG_US
    s_rtc_dog = CONFIG_BSP_RTC_PSEUDO_DOG_US;
    #endif
    
	// Enable 32KHz Clock
	if (!osc_is_ready(OSC_ID_OSC32)) {
    	osc_enable(OSC_ID_OSC32);
    	osc_wait_ready(OSC_ID_OSC32);
	}
	
    // GPIO to Blink
    gpio_configure_pin(BSP_RTC_LED, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);

    // init AST to counter mode
    ast_init_counter(&AVR32_AST, AST_OSC_32KHZ, AST_PSEL, 0);
    
    // Register the interrupt
    INTC_register_interrupt(&ast_int_handler, AVR32_AST_PER_IRQ, AVR32_INTC_INT0);
    
    // set up periodic alarm
    avr32_ast_pir0_t pir = { .insel = AST_PSEL };
    ast_set_periodic0_value(&AVR32_AST, pir);
    ast_enable_periodic_interrupt(&AVR32_AST,0);
	ast_enable_async_wakeup(&AVR32_AST, AVR32_AST_PER0_MASK);
    ast_enable_periodic0(&AVR32_AST);
    
    #ifdef CONFIG_STI_CMD_CLOCK
    // Register the pti command
    bsp_sti_register_command(&clock_command);
    #endif
    
    // start
    ast_enable(&AVR32_AST);
}

uint32_t bsp_rtc_get_clock(void)
{
    return s_clock;
}

uint32_t bsp_rtc_get_clock_us(void)
{
    return s_clock_us + cpu_cy_2_us(Get_sys_count() - s_tick_cycle, sysclk_get_cpu_hz());
}

void bsp_rtc_set_clock(uint32_t val)
{
    if (s_clock != val)
    {
        s_clock = val;
        
        #ifdef CONFIG_BSP_ENABLE_TKVS
        bsp_tkvs_publish_immed(BSP_TKVS_SRC_CLOCK, BSP_CLOCK_EVENT_UPDATE, 0);
        #endif
    }
}

uint32_t bsp_rtc_get_uptime(void)
{
    return 1000 * s_uptime + bsp_rtc_get_clock_us() / 1000;
}

uint32_t bsp_rtc_get_ticks(void)
{
    return s_up_ticks;
}

void bsp_rtc_idle_kick(void)
{
    #ifdef CONFIG_BSP_RTC_PSEUDO_DOG_US
    s_rtc_dog = CONFIG_BSP_RTC_PSEUDO_DOG_US;
    #endif
}

#endif // CONFIG_BSP_ENABLE_RTC