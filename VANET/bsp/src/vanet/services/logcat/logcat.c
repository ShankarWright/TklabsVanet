/**
 *	@file	logcat.c
 *
 *	@brief	Logging Service
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

#ifdef CONFIG_BSP_ENABLE_LOGCAT

static bool s_initialized = false;
static bsp_circ_buffer_t s_logcatq;
static uint8_t s_logcat_buffer[8192];
static bsp_circ_iter_t s_current_positon;
static uint16_t s_logcat_store_mask = 0xffff;			// store everything logged
static uint16_t s_logcat_cat_mask;
static uint8_t s_logcat_termios = 0xff;

// use interrupt locking instead of OS locking so that print() can be called from ISRs
#define logcat_lock_entry()             irqflags_t _flags
#define logcat_lock()                   _flags = cpu_irq_save()
#define logcat_unlock()                 cpu_irq_restore(_flags)
  
// header in queue for logcat entries... followed by 'size' bytes of text
typedef struct
{
	uint32_t uptime;
	uint16_t mask;
	uint8_t size;
} entry_t;

static void logcat_handler(int argc, char** argv, uint8_t port)
{
	// Reset default mask
	if (bsp_cp_valid())
		s_logcat_cat_mask = bsp_cp_get_field(logcat_print_mask);
	else
		s_logcat_cat_mask = BSP_LOGCAT_DEFAULT;
		
	if (argc > 1 && !strcmp(argv[1],"-c"))
	{
		bsp_circ_clear(&s_logcatq);
	}
	else
	{
		if (argc > 1) s_logcat_cat_mask = strtoul(argv[1],0,16);
		s_current_positon = bsp_circ_begin(&s_logcatq);
	}

	bsp_termios_printf(port, "Logcat Level: %04X\r\n", s_logcat_store_mask & s_logcat_cat_mask);
}

static bsp_sti_command_t logcat_command =
{
	.name = "logcat",
	.handler = &logcat_handler,
	.minArgs = 0,
	.maxArgs = 1,
	STI_HELP("logcat [-c|<level>]                   Show or clear the log")
};

static void logcat_idle_display(void)
{
    if (s_logcat_termios != 0xff && !bsp_circ_eof(&s_logcatq,s_current_positon))
    {
        entry_t entry;
        char* buf = bsp_malloc(256);
        
        for (int i = 10; (i > 0 || !OSRunning) && !bsp_circ_eof(&s_logcatq,s_current_positon); )
        {
            // we want a complete message before we do anything
            if (bsp_circ_peek(&s_logcatq, s_current_positon, &entry, sizeof(entry)) >= sizeof(entry_t))
            {
                s_current_positon = bsp_circ_adv(&s_logcatq, s_current_positon, sizeof(entry_t));
                if (bsp_circ_peek(&s_logcatq, s_current_positon, buf, entry.size) >= entry.size)
                {
                    // we have a complete message in the queue!
                    s_current_positon = bsp_circ_adv(&s_logcatq, s_current_positon, entry.size);
                    
                    // should we display it?
                    if (entry.mask & s_logcat_cat_mask)
                    {
                        bsp_termios_printf(s_logcat_termios, "%08u %04X %s\r\n", entry.uptime, entry.mask, buf);
                        bsp_termios_flush(s_logcat_termios);
                        i--;
                    }
                }
            }
        }
        
        bsp_free(buf);
    }
}

void bsp_logcat_init(void)
{
	bsp_circ_init(&s_logcatq, s_logcat_buffer, sizeof(s_logcat_buffer));
	
	s_logcat_cat_mask = BSP_LOGCAT_DEFAULT;
    s_current_positon = bsp_circ_begin(&s_logcatq);
	s_initialized = true;
    
    bsp_idle_register_idle_function(logcat_idle_display, BSP_IDLE_ALWAYS);
	
	bsp_sti_register_command(&logcat_command);
}

#ifdef CONFIG_BSP_ENABLE_CODEPLUG
void bsp_logcat_codeplug_ready(void)
{
	if (bsp_cp_valid())
	{
		s_logcat_store_mask = bsp_cp_get_field(logcat_store_mask);
		s_logcat_cat_mask = bsp_cp_get_field(logcat_print_mask);
	}		
}
#endif

void bsp_logcat_set_termios(uint8_t t)
{
    s_logcat_termios = t;
}

static void logprint(uint16_t mask, const char *msg, uint32_t msglen)
{
	entry_t entry;
	bool adv;
	    
	// Only actually log it if it matches our logging mask (runtime disable logcats!)
	if (s_initialized && mask & s_logcat_store_mask)
	{
	    uint16_t needed = sizeof(entry_t) + msglen;
    
		logcat_lock_entry();
		logcat_lock();
        
		// remove old messages until we have enough space for the new one
		while (bsp_circ_free(&s_logcatq) < needed)
		{
			adv = s_current_positon == bsp_circ_begin(&s_logcatq); // don't let the curr_position get swallowed

			if (adv) s_current_positon = bsp_circ_adv(&s_logcatq,s_current_positon,sizeof(entry));
			bsp_circ_read(&s_logcatq,&entry,sizeof(entry));
			if (adv) s_current_positon = bsp_circ_adv(&s_logcatq,s_current_positon,entry.size);
			bsp_circ_read(&s_logcatq,0,entry.size);
		}
		    
		entry.mask = mask;
		entry.uptime = OSTimeGet() * CONFIG_BSP_RTC_TICK_HZ; // bsp_rtc_get_uptime();
		entry.size = msglen;
		bsp_circ_write(&s_logcatq, &entry, sizeof(entry));
		bsp_circ_write(&s_logcatq, (const uint8_t*)msg, msglen);
        
		logcat_unlock();
	}
}

void bsp_logcat_print(uint16_t mask, const char *msg)
{
    logprint(mask,msg,strlen(msg) + 1);
}

void bsp_logcat_printf(uint16_t mask, const char* format, ...)
{
    char* buf = bsp_malloc(256);
    uint32_t len;
	va_list va;
	va_start(va,format);
	len = dlib_vsnprintf(buf,256,format,va);
	logprint(mask,buf,len+1);
    bsp_free(buf);
}

#define IS_PRINTABLE(x)         (((x) >= 0x20) && ((x) <= 0x7E))
void bsp_logcat_dump(uint16_t mask, const uint8_t *addr, int len)
{
    int i = 0;
    char ascii[16];
    char* buf = bsp_malloc(128);
    char* str = buf;
    
    while (len > 0)
    {
	    char c = *addr++;
	    bsp_util_char_hex_to_str(str, c); str += 2;
	    *str++ = ' ';
	    
	    ascii[i] = IS_PRINTABLE(c) ? c : '.';
	    if (++i == 16)
	    {
		    memcpy(str,"   ",3); str += 3;
		    memcpy(str,ascii,16); str += 16;
		    *str = 0;
		    logprint(mask, buf, (uint32_t)(str - buf + 1));
		    str = buf;
		    i = 0;
	    }
	    --len;
    }
    if (i > 0)
    {
	    while (i < 16)
	    {
		    memcpy(str,"   ",3);
		    str += 3;
		    ascii[i] = ' ';
		    i++;
	    }
	    memcpy(str,"   ",3); str += 3;
	    memcpy(str,ascii,16); str += 16;
	    *str = 0;
	    logprint(mask, buf, (uint32_t)(str - buf + 1));
    }
    
    bsp_free(buf);
}

void bsp_logcat_reset_dump(void)
{
    entry_t entry;
    bsp_circ_iter_t i;
    char* buf = bsp_malloc(256);
        
    for (i = bsp_circ_begin(&s_logcatq); !bsp_circ_eof(&s_logcatq,i); )
    {
        if (bsp_circ_peek(&s_logcatq,i,&entry,sizeof(entry_t)) < sizeof(entry_t)) break;
        i = bsp_circ_adv(&s_logcatq,i,sizeof(entry_t));

        if (bsp_circ_peek(&s_logcatq,i,buf,entry.size) < entry.size) break;
        i = bsp_circ_adv(&s_logcatq,i,entry.size);
            
		bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "%08u %04X %s\r\n", entry.uptime, entry.mask, buf);
    }
    
    bsp_free(buf);
}

#endif // CONFIG_BSP_ENABLE_LOGCAT