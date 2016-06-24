/**
 *	@file	sti_cmds.c
 *
 *	@brief	System Test Interface (STI) Internal Commands
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
#include <string.h>

#ifdef CONFIG_BSP_ENABLE_STI

// From sti.c
extern bsp_sti_command_t *s_commandListHead;
extern char s_history[STI_CMD_HISTORY_LEN][MAX_COMMAND_LENGTH+1];

extern void bsp_sti_install_builtin(void);

/* -----------
 *  HELP
 * ----------*/
static void help_handler(int argc, char** argv, uint8_t port)
{
    bsp_sti_command_t* nextCmd;
    
    if (argc > 1)
    {
        for (nextCmd = s_commandListHead; nextCmd; nextCmd = nextCmd->next)
        {
            if (!strcasecmp(argv[1],nextCmd->name))
            {
				if (nextCmd->help)
					bsp_termios_write_str(port, nextCmd->help);
				else
					bsp_termios_write_str(port, "No help available");
				
                bsp_termios_write_str(port, "\r\n");
                return;
            }
        }
        
        bsp_termios_write_str(port, "Unknown command: ");
		bsp_termios_write_str(port, argv[1]);
		bsp_termios_write_str(port, "\r\n");
    }
    else
    {
        bsp_termios_write_str(port, "Available Commands:\r\n");
        for (nextCmd = s_commandListHead; nextCmd; nextCmd = nextCmd->next)
        {
            bsp_termios_write_str(port, nextCmd->name);
            bsp_termios_write_str(port, "\r\n");
        }
        
        bsp_termios_write_str(port, "\r\nType 'help <cmd>' to see usage on a command\r\n");

    }
}

static bsp_sti_command_t help_command =
{
    .name = "help",
    .handler = &help_handler,
    .minArgs = 0,
    .maxArgs = 1,
    STI_HELP("help                                  List available commands\r\n"
             "help <command>                        Show usage for command")
};

#ifdef CONFIG_STI_CMD_RESET
/* -----------
 *  RESET
 * ----------*/
static void reset_handler(int argc, char** argv, uint8_t port)
{
    uint16_t code = BSP_RESET_SOFT;
    if (argc > 1) code = strtoul(argv[1],0,16);
    
    // check for some codes we recognize
    switch (code)
    {
    case BSP_RESET_WATCHDOG:
        while (true); // block to let the watchdog expire
        
    case 0x8000:
    case 0x8034:
        print_dbg_hex(*(uint32_t*)1); // test a cpu exception
        break;

    default:
        bsp_reset(code);
        break;
    }
}

static bsp_sti_command_t reset_command =
{
    .name = "reset",
    .handler = &reset_handler,
    .minArgs = 0,
    .maxArgs = 1,
    STI_HELP("reset                                 Reset the chip using OCD")
};

#endif // CONFIG_STI_CMD_RESET

/* -----------
 *  VERSION
 * ----------*/
static void version_handler(int argc, char** argv, uint8_t port)
{
	bsp_sti_show_build_info(port);
}

static bsp_sti_command_t version_command =
{
	.name = "version",
	.handler = &version_handler,
	.minArgs = 0,
	.maxArgs = 0
};

/* -----------
 *  HISTORY
 * ----------*/
static void history_handler(int argc, char** argv, uint8_t port)
{
	bsp_termios_write_str(port, "\r\n");
	for(int j=0; j<STI_CMD_HISTORY_LEN; j++)
	{
		if (s_history[j][0] == '\0')
			break;
			
		bsp_termios_printf(port, "%d : %s\r\n", j, s_history[j]);
	}

}

static bsp_sti_command_t history_command =
{
	.name = "history",
	.handler = &history_handler,
	.minArgs = 0,
	.maxArgs = 0
};

/* -----------
 *  SYSINFO
 * ----------*/
static void sysinfo_handler(int argc, char** argv, uint8_t port)
{      
    bsp_termios_write_str(port, "Calibration Data\r\n");
    bsp_termios_write_str(port, "    Oscillator       : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800200);
    bsp_termios_write_str(port, "    ADC Core         : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800204);
    bsp_termios_write_str(port, "    ADC S/H          : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800208);
    bsp_termios_write_str(port, "    DAC0A            : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x8080020c);
    bsp_termios_write_str(port, "    DAC0B            : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800210);
    bsp_termios_write_str(port, "    DAC1A            : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800214);
    bsp_termios_write_str(port, "    DAC1B            : ");   bsp_termios_printf(port, "%08X\r\n", *(uint32_t *)0x80800218);
    bsp_termios_write_str(port, "    Serial Number    : ");   
	for (int i=0; i<15; i++)
		bsp_termios_printf(port, "%02X ", *(uint8_t *)(0x80800284+i));
	bsp_termios_write_str(port, "\r\n");
    bsp_termios_write_str(port, "System Clocks\r\n");
    bsp_termios_write_str(port, "    Main System      : ");   bsp_termios_printf(port, "%u\r\n", sysclk_get_main_hz());
    bsp_termios_write_str(port, "    CPU (HSB)        : ");   bsp_termios_printf(port, "%u\r\n", sysclk_get_cpu_hz());
    bsp_termios_write_str(port, "    Peripheral Bus A : ");   bsp_termios_printf(port, "%u\r\n", sysclk_get_pba_hz());
    bsp_termios_write_str(port, "    Peripheral Bus B : ");   bsp_termios_printf(port, "%u\r\n", sysclk_get_pbb_hz());
    bsp_termios_write_str(port, "    Peripheral Bus C : ");   bsp_termios_printf(port, "%u\r\n", sysclk_get_pbc_hz());
    bsp_termios_write_str(port, "System Memory\r\n");
    bsp_termios_write_str(port, "    SRAM             : ");   
	bsp_termios_printf(port, "%08X - %08X\r\n", AVR32_SRAM_ADDRESS, AVR32_SRAM_ADDRESS + AVR32_SRAM_SIZE - 1);
    bsp_termios_write_str(port, "    Flash            : ");   
	bsp_termios_printf(port, "%08X - %08X\r\n", AVR32_FLASH_ADDRESS, AVR32_FLASH_ADDRESS + AVR32_FLASH_SIZE - 1);
    bsp_termios_write_str(port, "    User Flash       : ");
    bsp_termios_printf(port, "%08X - %08X\r\n", AVR32_FLASHC_USER_PAGE_ADDRESS, AVR32_FLASHC_USER_PAGE_ADDRESS + AVR32_FLASHC_USER_PAGE_SIZE - 1);
    bsp_termios_write_str(port, "    HSB SRAM         : ");   
	bsp_termios_printf(port, "%08X - %08X\r\n", AVR32_HRAMC0_ADDRESS, AVR32_HRAMC0_ADDRESS + AVR32_HRAMC0_SIZE - 1);     
}

static bsp_sti_command_t sysinfo_command =
{
    .name = "sysinfo",
    .handler = &sysinfo_handler,
    .minArgs = 0,
    .maxArgs = 0,
    STI_HELP("sysinfo                               Show system info\r\n")
};

void bsp_sti_install_builtin(void)
{
	bsp_sti_register_command(&help_command);
	bsp_sti_register_command(&history_command);
	bsp_sti_register_command(&version_command);
#ifdef CONFIG_STI_CMD_RESET
	bsp_sti_register_command(&reset_command);
#endif // CONFIG_STI_CMD_RESET
	bsp_sti_register_command(&sysinfo_command);
	
	// Termios is a requirement for STI so we can safely assume this exists
	bsp_sti_register_command(&termios_command);
}

#endif // CONFIG_BSP_ENABLE_STI