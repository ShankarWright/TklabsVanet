/**
 *    @file    reset.c
 *
 *    @brief    BSP Reset Handling
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of CWSI, LLC. Sunrise FL, USA
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
#include "ucos_ii.h"

enum
{
    SECTION_DATA = 0,
    SECTION_STACK
};

typedef struct 
{
    uint8_t type;
    uint8_t pad[3];
    uint32_t base;
    uint32_t size;
} dump_section_t;

typedef struct 
{
    char version[32];
    char build_date[32];
    uint32_t exception_code;
    uint32_t exception_addr;
} system_dump_t;

#ifdef CONFIG_BSP_ENABLE_RAM_DUMP

static inline uint32_t bsp_util_word_to_little_endian(uint32_t word)
{
    return ((word & 0x000000ff) << 24)
            | ((word & 0x0000ff00) << 8)
            | ((word & 0x00ff0000) >> 8)
            | ((word & 0xff000000) >> 24);
}

/// Convert to little-endian
static inline uint16_t bsp_util_short_to_little_endian(uint16_t word)
{
    return (uint16_t) (((word & 0x00ff) << 8) | ((word & 0xff00) >> 8));
}

static void mem_dump(const void* _ptr, uint32_t size)
{
    const uint8_t* ptr = (const uint8_t*) _ptr;
    while (size > 0)
    {
        usart_putchar(DBG_USART,*ptr++);
        --size;
        
        #ifdef CONFIG_BSP_HW_WDT_PIN
        gpio_toggle_pin(CONFIG_BSP_HW_WDT_PIN); // external hardware watchdog
        #endif
    }
}

static void ram_dump_data(uint32_t base, uint32_t size)
{
    dump_section_t section;
    section.type = SECTION_DATA;
    section.base = bsp_util_word_to_little_endian(base);
    section.size = bsp_util_word_to_little_endian(size);
    
    mem_dump(&section, sizeof(section));
    mem_dump((void*)base, size);
}

static void ram_dump_stack(const char* name, uint32_t curr, uint32_t base, uint32_t size)
{
    dump_section_t section;
    section.type = SECTION_STACK;
    section.base = bsp_util_word_to_little_endian(base);
    section.size = bsp_util_word_to_little_endian(size);
    curr = bsp_util_word_to_little_endian(curr);
    
    mem_dump(&section, sizeof(section));
    mem_dump(name,strlen(name));
    usart_putchar(DBG_USART,0);
    mem_dump(&curr,4);
    mem_dump((void*)base, size);
}

static void ram_dump_other(void)
{
    system_dump_t sys;
    memset(&sys,0,sizeof(sys));
    strcpy(sys.version,VERSION_ABOUT " (Build " ASTRINGZ(VERSION_SOFTWARE_BUILD) ")");
    strcpy(sys.build_date,VERSION_BUILD_DATE);
    sys.exception_code = bsp_util_word_to_little_endian(scif_read_gplp(0) & 0xffff);
    sys.exception_addr = bsp_util_word_to_little_endian(scif_read_gplp(1));

    dump_section_t section;
    section.type = SECTION_DATA;
    section.base = bsp_util_word_to_little_endian(0xffff0000);
    section.size = bsp_util_word_to_little_endian(sizeof(sys));

    mem_dump(&section, sizeof(section));
    mem_dump(&sys, sizeof(sys));
}

#endif // CONFIG_BSP_ENABLE_RAM_DUMP

static void print_exception(char* msg, uint16_t code, uint32_t addr)
{
    print_dbg(msg);
    print_dbg(": ");
    print_dbg_short_hex(code);
    print_dbg_char(' ');
    print_dbg_hex(addr);
    print_dbg("\r\n");
}

static void ram_dump(void)
{
    #ifdef CONFIG_BSP_ENABLE_RAM_DUMP

    // stop the watchdog
    wdt_disable();

    // wait for command
    while (true)
    {
        int r, c;
        while ((r = usart_read_char(DBG_USART, &c)) == USART_RX_EMPTY)
        {
            #ifdef CONFIG_BSP_HW_WDT_PIN
            gpio_toggle_pin(CONFIG_BSP_HW_WDT_PIN); // external hardware watchdog
            #endif
        }
        
        if (r != USART_SUCCESS)
        {
            init_dbg_rs232(sysclk_get_pba_hz());
            bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "RS232 Error: %d\r\n", r);
        }
        else if (c == 'h')
        {
            bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT,
                      "Welcome to Reset Dump\r\n"
                      "@    - Dump ram\r\n"
                      "c    - Exception code\r\n"
                      "r    - Reset\r\n"
                      "l    - Logcat\r\n"
                      #ifdef CONFIG_BSP_ENABLE_BUFFERS
                      "b    - Buffers\r\n"
                      #endif
                      #ifdef CONFIG_BSP_ENABLE_OSTRACKER
                      "s    - Stack usage\r\n"
                      "o    - OS configuration\r\n"
                      "w    - Task switch history\r\n"
                      #endif
                      );
        }
        else if (c == '@')
        {
            // dump RAM sections
            ram_dump_data(AVR32_SRAM_ADDRESS,AVR32_SRAM_SIZE);
            ram_dump_data(AVR32_FLASHC_USER_PAGE_ADDRESS,AVR32_FLASHC_USER_PAGE_SIZE);
            ram_dump_data(AVR32_HRAMC0_ADDRESS,AVR32_HRAMC0_SIZE);
            
            // dump stacks
            OS_TCB tcb;
            uint8_t i;
            for (i=0; i<OS_LOWEST_PRIO; i++)
            {
                if (OSTaskQuery(i,&tcb) == OS_ERR_NONE)
                {
                    ram_dump_stack((const char*) tcb.OSTCBTaskName,(uint32_t)tcb.OSTCBStkPtr,
                                   (uint32_t)tcb.OSTCBStkBottom,tcb.OSTCBStkSize*sizeof(OS_STK));
                }
            }
            
            // dump info thats not in ram
            ram_dump_other();
            
            ram_dump_data(0,0); // signal the end
        }
        else if (c == 'l')
        {
            bsp_logcat_reset_dump();
        }
        #ifdef CONFIG_BSP_ENABLE_BUFFERS
        else if (c == 'b')
        {
            bsp_buffer_dump(BSP_TERMIOS_RAW_DEBUG_PORT);
        }
        #endif
        else if (c == 'c')
        {
            print_exception("Fatal Exception", scif_read_gplp(0) & 0xffff, scif_read_gplp(1));
        }
        #ifdef CONFIG_BSP_ENABLE_OSTRACKER
        else if (c == 's')
        {
            bsp_print_stack_usage(BSP_TERMIOS_RAW_DEBUG_PORT);
        }
        else if (c == 'o')
        {
            bsp_print_os_config(BSP_TERMIOS_RAW_DEBUG_PORT);
        }
        else if (c == 'w')
        {
            bsp_print_stack_switches(BSP_TERMIOS_RAW_DEBUG_PORT);
        }
        #endif
        else if (c == 'r')
        {
            // reset
            break;
        }
        else
        {
            bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "'h' for help\r\n");
        }
    }
    
    #endif
}

static void clear_scif_regs(void)
{
    scif_write_gplp(0, 0);
    scif_write_gplp(1, 0);
}

static bool is_scif_reg_empty(void)
{
    return scif_read_gplp(0) == 0 && scif_read_gplp(1) == 0;
}

static void cpu_exception(uint16_t code, uint32_t addr)
{
    // stop everything
    cpu_irq_disable();
    
    // the OS is no longer running
    OSRunning = false;
    
    // check for rolling reset
    if (!is_scif_reg_empty())
    {
        // we got another exception during reset handling, don't bother trying to do stuff
        print_exception("Rolling Exception", code, addr);
        bsp_delay(10);
        reset_do_soft_reset();
    }
    
    // check for special case intentional reset
    if (code == BSP_RESET_SOFT)
    {
        #ifdef CONFIG_BSP_ENABLE_TERMIOS
        bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Resetting board...\r\n");
        bsp_delay(10);
        #endif
        reset_do_soft_reset();
    }
    
    // save the reset reason in the SCIF in case we reset again
    scif_write_gplp(0, code);
    scif_write_gplp(1, addr);
    
    // make sure we don't watchdog during this routine
    wdt_clear();
    
    #ifdef CONFIG_BSP_ENABLE_BUZZER
    // out of pity for the developer
    bsp_buzzer_stop();
    #endif
    
    // blurp the exception out the debug port
    print_exception("Fatal Exception", code, addr);
        
    if (code == BSP_RESET_ROLLING_TEST)
    {
        // cause an exception on purpose for testing
        print_dbg_hex(*(uint32_t*)1);
    }        
    
    // wait for ram dump if enabled
    ram_dump();
    
    // clear the  scif so we know we aren't rolling
    clear_scif_regs();
    
    reset_do_soft_reset();
}

static void print_reset_cause(reset_cause_t cause)
{
    #ifdef CONFIG_BSP_ENABLE_TERMIOS
    #ifdef CONFIG_BSP_ENABLE_RESET_DECODE
    bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "\r\n\r\nReset Cause  : ");
    if (cause & RESET_CAUSE_BOD_CPU)    bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "CPU Brown Out");
    if (cause & RESET_CAUSE_BOD_IO)     bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "I/O Brown Out");
    if (cause & RESET_CAUSE_CPU_ERROR)  bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "CPU Error");
    if (cause & RESET_CAUSE_EXTRST)     bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "External");
    if (cause & RESET_CAUSE_JTAG)       bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "JTAG");
    if (cause & RESET_CAUSE_OCD)        bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "On Chip Debug");
    if (cause & RESET_CAUSE_POR)        bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Power-On");
    if (cause & RESET_CAUSE_SLEEP)      bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Shutdown Sleep Mode");
    if (cause & RESET_CAUSE_SOFT)       bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Soft");
    if (cause & RESET_CAUSE_SPIKE)      bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Spike");
    if (cause & RESET_CAUSE_WDT)        bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "Watchdog");
    bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "\r\n");
    #else // CONFIG_BSP_ENABLE_RESET_DECODE
    bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "\r\n\r\nReset Cause  : %X\r\n", cause);
    #endif // CONFIG_BSP_ENABLE_RESET_DECODE
    #endif // CONFIG_BSP_ENABLE_TERMIOS
}

void bsp_reset_init(void)
{
    reset_cause_t cause = reset_cause_get_causes();
    
    if (cause & RESET_CAUSE_WDT)
    {
        cpu_exception(BSP_RESET_WATCHDOG,0);
    }
    else if (!is_scif_reg_empty())
    {
        // we're rolling
        print_exception("Fatal Exception", scif_read_gplp(0), scif_read_gplp(1));
    }
    
    // make sure the scif regs are clear
    clear_scif_regs();
        
    // Print Reset Reason
    print_reset_cause(cause);
}

void bsp_reset(bsp_reset_reason_t reason)
{
    uint32_t addr = 0;
    asm ("mov %0, lr" : "+r"(addr));
    addr -= 4; // we don't know where we came from, but assume it was a 4-byte rcall
    
    cpu_exception((uint16_t)reason, addr);
}

void bsp_exception(uint16_t vector, uint32_t addr)
{
    cpu_exception(BSP_EXCEPTION_BASE | vector, addr);
}

void bsp_stop(void)
{
    // stop everything
    cpu_irq_disable();
    wdt_disable();

    for (;;)
    {
        #ifdef CONFIG_BSP_HW_WDT_PIN
        gpio_toggle_pin(CONFIG_BSP_HW_WDT_PIN); // external hardware watchdog
        #endif
    }
}