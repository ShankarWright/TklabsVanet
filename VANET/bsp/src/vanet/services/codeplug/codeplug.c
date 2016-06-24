/**
 *	@file	codeplug.c
 *
 *	@brief	Codeplug
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

#include <string.h>		// memcmp
#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_CODEPLUG

__attribute__((__section__(".userpage")))
codeplug_t g_codeplug;

__attribute__((__section__(".userpage_xsum")))
static const volatile uint16_t s_cp_xsum;

bsp_codeplug_t *p_bsp_codeplug;
extern const bsp_codeplug_t bsp_defaults;

/// temporary storage to hold factory codeplug when we update defaults
bsp_factory_codeplug_t non_volatile;

static void update_checksum(void)
{
	uint16_t xsum = bsp_util_fletcher16((const uint8_t*)&g_codeplug, sizeof(g_codeplug));
	flashc_memcpy((void *)&s_cp_xsum,&xsum,2,true);
}

static void backup_codeplug(void)
{
	// TODO
}

static void restore_codeplug(void)
{
	// check backup codeplug
	
	// restore backup codeplug
	
	// correct the checksum
	update_checksum();
}

static void init_codeplug_from_defaults(void)
{
	bsp_cp_write((void *)&g_codeplug.bsp_codeplug, (void *)&bsp_defaults, sizeof(bsp_codeplug_t));
	//bsp_cp_write((void *)&g_codeplug.user_codeplug, (void *)appinfo_defaults(), sizeof(user_codeplug_t));
	update_checksum(); // force this in case it wasn't done before
}

bool bsp_cp_verify(void)
{
	// flag an invalid codeplug unless we find a valid one
	p_bsp_codeplug = NULL;
	
	// verify codeplug signature
	if (g_codeplug.bsp_codeplug.cp_signature == BSP_CP_SIGNATURE)
	{
		// Verify the checksum
		uint16_t xsum;
		xsum = bsp_util_fletcher16((const uint8_t*)&g_codeplug, sizeof(g_codeplug));
		if (xsum == s_cp_xsum)
		{
			p_bsp_codeplug = &g_codeplug.bsp_codeplug;
			//p_user_codeplug = &g_codeplug.user_codeplug;
		}
		else
		{
			print_dbg("Checksum Error");
		}
	}
	else
	{
		print_dbg("Signature Error ");
		print_dbg_hex(g_codeplug.bsp_codeplug.cp_signature);
	}
	
	if (p_bsp_codeplug == NULL)     // codeplug was bad
	{
		print_dbg(" Codeplug bad - using defaults!\r\n");
		p_bsp_codeplug = &bsp_defaults;
		//p_user_codeplug = appinfo_defaults();
	}

	#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_codeplug_ready();
	#endif

	return bsp_cp_valid();
}

void bsp_cp_write(void* dst, const void* src, size_t size)
{
	// allow directed writes to user page - writes via macros when codeplug is bad will not happen
	if (bsp_util_address_in_user_page(dst))
	{
		if (memcmp(dst,src,size))
		{
			flashc_memcpy(dst,src,size,true);
			update_checksum();
		}
	}
}

bool bsp_cp_valid(void)
{
	return (p_bsp_codeplug == &g_codeplug.bsp_codeplug);
}

bool bsp_cp_verify_version()
{
	if ( bsp_cp_valid() &&
		(g_codeplug.bsp_codeplug.factory.factory_version == bsp_defaults.factory.factory_version) &&
		(g_codeplug.bsp_codeplug.defaults_version == bsp_defaults.defaults_version)
		)
	{
		return true;
	}
	else
	{
		// invalid codeplug or version is wrong
		return false;
	}
}

#ifdef CONFIG_STI_CMD_CP

// defaults.h
extern void cp_print_field(uint8_t field);
extern uint8_t cp_print_num_fields(void);
// defaults.h

static void cp_handler(int argc, char** argv, uint8_t port)
{
	int             i;
	uint32_t        int_data;
	unsigned char   hex_data[16];
	bool            reboot = false;
	uint8_t         *p_cp;
	bool			partial_match = false;
	char			factory_name[32];
	
	if (!strcasecmp(argv[1],"backup"))
	{
		backup_codeplug();
	}
	else if (!strcasecmp(argv[1],"restore"))
	{
		restore_codeplug();
		reboot = true;
	}
	else if (!strcasecmp(argv[1], "nuke"))
	{
		init_codeplug_from_defaults();
		reboot = true;
	}
	else if (!strcasecmp(argv[1], "defaults"))
	{
		if (bsp_cp_valid())
		{
			// make a copy of the stuff we want to save!
			memcpy((uint8_t *)&non_volatile, &g_codeplug, sizeof(bsp_factory_codeplug_t));
			//print_dbg_array((uint8_t *)&non_volatile, sizeof(bsp_factory_codeplug_t));
			
			// write new data from current build
			init_codeplug_from_defaults();
			//print_dbg_array((uint8_t *)&g_codeplug, sizeof(bsp_factory_codeplug_t));
			
			// overwrite the saved data
			bsp_cp_write((void *)&g_codeplug.bsp_codeplug, (void *)&non_volatile, sizeof(bsp_factory_codeplug_t));
			//print_dbg_array((uint8_t *)&g_codeplug, sizeof(bsp_factory_codeplug_t));
			
			// and mark it good!
			update_checksum();
			reboot = true;
		}
		else
		{
			bsp_termios_write_str(port, "Codeplug Invalid - Can't set new defaults!\r\n");
		}
	}
	else if (!strcasecmp(argv[1], "status"))
	{
		if (bsp_cp_valid())
			bsp_termios_write_str(port, "Codeplug Valid\r\n");
		else
			bsp_termios_write_str(port, "Codeplug Invalid - Using Defaults!\r\n");
		
		if (bsp_cp_verify_version())
			bsp_termios_write_str(port, "Codeplug Version OK\r\n");
		else
			bsp_termios_write_str(port, "Codeplug Version Mismatch!\r\n");
		
		bsp_termios_printf(port, "  Factory Version: Found %d Expected %d\r\n", 
			g_codeplug.bsp_codeplug.factory.factory_version, bsp_defaults.factory.factory_version);
		bsp_termios_printf(port, "  Default Version: Found %d Expected %d\r\n",
			g_codeplug.bsp_codeplug.defaults_version, bsp_defaults.defaults_version);
		
		if (argc == 3 && !strncasecmp(argv[2], "verbose", 1))
		{
			bsp_termios_write_str(port, "\r\n");
			bsp_termios_printf(port, "BSP Factory Size %d\r\n", sizeof(bsp_factory_codeplug_t));
			//print_dbg_array((uint8_t *)&g_codeplug, sizeof(bsp_factory_codeplug_t));
			bsp_termios_printf(port, "BSP Size %d\r\n", sizeof(bsp_codeplug_t) - sizeof(bsp_factory_codeplug_t));
			//print_dbg_array((uint8_t *)&g_codeplug + sizeof(bsp_factory_codeplug_t), sizeof(bsp_codeplug_t) - sizeof(bsp_factory_codeplug_t));
			//print_dbg("App Size: "); print_dbg_int(sizeof(user_codeplug_t)); print_dbg("\r\n");
			//print_dbg_array((uint8_t *)&g_codeplug + sizeof(bsp_codeplug_t), sizeof(user_codeplug_t));
			bsp_termios_write_str(port, "\r\n");
		}
		
	}
	else if (!strcasecmp(argv[1], "get"))
	{
		if (argc == 3)
		{
			// match 'cp get power_poll_time' as well as 'cp get power' which will return anything starting with 'power'
			for (i=0; i<cp_print_num_fields(); i++)
			{
				if (!strcasecmp(argv[2], cp_fields[i].name) || !strncasecmp(argv[2], cp_fields[i].name, strlen(argv[2])))
				{
					cp_print_field(i);
					// if we have an exact match exit
					if (!strcasecmp(argv[2], cp_fields[i].name))
						break;
					else
						partial_match = true;
				}
			}
			if (!partial_match && i == cp_print_num_fields())
			{
				bsp_termios_printf(port, "cp get - unknown field %s\r\n", argv[2]);
			}
		}
		else
		{
			if (!bsp_cp_valid())
				bsp_termios_write_str(port, "Warning: Codeplug Invalid - These Are Defaults!\r\n");
			
			for (i=0; i<cp_print_num_fields(); i++)
			{
				cp_print_field(i);
				OSTimeDly(1);
			}
		}
	}
	else if (!strcasecmp(argv[1], "set"))
	{
		if (argc == 4)
		{
			// also search via factory.xxx
			strcpy(factory_name, "factory.");
			strcat(factory_name, argv[2]);
			for (i=0; i<cp_print_num_fields(); i++)
			{
				// match xxx or factory.xxx
				if (!strcasecmp(argv[2], cp_fields[i].name) || !strcasecmp(factory_name, cp_fields[i].name))
				{
					// grab the right structure
					//p_cp = cp_fields[i].app ? (uint8_t *)p_user_codeplug : (uint8_t *)p_bsp_codeplug;
					p_cp = (uint8_t *)p_bsp_codeplug;
					
					if (cp_fields[i].format == CP_FIELD_HEX)
					{
						// input value is in hex - input is 2* # of bytes
						if (strlen(argv[3]) == cp_fields[i].size * 2)
						{
							if (!bsp_cp_valid()) 
								bsp_termios_write_str(port, "Warning: Codeplug Invalid - This Won't Do Anything!\r\n");
							bsp_util_decode_hex_string(argv[3], hex_data, cp_fields[i].size);
							bsp_cp_write(p_cp + cp_fields[i].offset, hex_data, cp_fields[i].size);
							cp_print_field(i);
							reboot = true;
						}
						else
						{
							bsp_termios_printf(port, "'cp set %s' requires %d bytes of data\r\n", argv[2], cp_fields[i].size);
						}
					}
					else if (cp_fields[i].format == CP_FIELD_INT || cp_fields[i].format == CP_FIELD_UINT)
					{
						if (!bsp_cp_valid()) 
							bsp_termios_write_str(port, "Warning: Codeplug Invalid - This Won't Do Anything!\r\n");
						int_data = strtol(argv[3],0,10);
						bsp_cp_write(p_cp + cp_fields[i].offset, (uint8_t *)&int_data + (4 - cp_fields[i].size), cp_fields[i].size);
						cp_print_field(i);
						reboot = true;
					}
					break;
				}
			}
			if (i == cp_print_num_fields())
			{
				bsp_termios_printf(port, "cp set - unknown field %s\r\n", argv[2]);
			}
		}
		else
		{
			bsp_termios_write_str(port, "cp set WHAT?\r\n");
		}
	}
	else
	{
		bsp_termios_write_str(port, "Invalid arguments\r\n");
	}
	
	if (reboot) 
		bsp_termios_write_str(port, "Changes require reboot to take effect");
}

static bsp_sti_command_t cp_command =
{
	.name = "cp",
	.handler = &cp_handler,
	.minArgs = 1,
	.maxArgs = 3,
	STI_HELP(
	"cp status                             Show codeplug status\r\n"
	"cp backup                             Backup the codeplug\r\n"
	"cp restore                            Restore codeplug from backup\r\n"
	"cp defaults                           Set codeplug to defaults\r\n"
	"\r\n"
	"cp get <field>                        Show the codeplug field (leave off <field> to display all fields\r\n"
	"cp set <field>                        Set the codeplug field\r\n"
	)
};
#endif // CONFIG_STI_CMD_CP

void bsp_cp_init(void)
{
	bsp_cp_verify();
	
	// Register the sti commands
	#ifdef CONFIG_STI_CMD_CP
	bsp_sti_register_command(&cp_command);
	#endif
}


#endif // CONFIG_BSP_ENABLE_CODEPLUG