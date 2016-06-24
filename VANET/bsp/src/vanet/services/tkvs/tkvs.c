/**
 *	@file	tkvs.c
 *
 *	@brief	tkLABS Virtual Source
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

typedef struct 
{
    uint8_t source;
    uint16_t event_mask;
    uint8_t subscribed_os_pri;
    OS_EVENT* queue;
    uint32_t hits;
} subscription_t;

static subscription_t s_subscriptions[CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS];

static uint32_t s_alloc_cnt, s_free_cnt;

#ifdef CONFIG_BSP_TKVS_ENABLE_1STICK
static bsp_tkvs_timer_t tkvs_1s_tick;
#endif // CONFIG_BSP_TKVS_ENABLE_1STICK

#ifdef CONFIG_STI_CMD_TKVS
static void tkvs_handler(int argc, char** argv, uint8_t port)
{
    int n, j, k;
    uint32_t total_hits = 0;
    subscription_t top5[5];
    subscription_t *subs;
    int num_subs;
    INT8U *task_name_ptr;
    INT8U perr;
    
    // -v we display ALL subscriptions
    if (argc == 2 && !strncmp(argv[1], "-v", 2))
    {
        num_subs = CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS;
        subs = s_subscriptions;
        
        for (n=0; n<CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS && s_subscriptions[n].queue; n++)
        {
            total_hits += s_subscriptions[n].hits;
        }
    }
    else
    {    
        // otherwise - just the top 5
        num_subs = 5;
        subs = top5;
        memset(top5,0,sizeof(top5));
    
        // count the subscriptions
        for (n=0; n<CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS && s_subscriptions[n].queue; n++)
        {
            total_hits += s_subscriptions[n].hits;
        
            for (j=0; j<5; j++)
            {
                if (s_subscriptions[n].hits > top5[j].hits)
                {
                    for (k=4; k>j; k--) top5[k] = top5[k-1];
                    top5[j] = s_subscriptions[n];
                    break;
                }
            }
        }
    }
    
	bsp_termios_printf(port, "Subscriptions: %u\r\nTotal Hits: %u\r\n", n, total_hits);
    for (j=0; j<num_subs; j++)
    {
        if (subs[j].queue)
        {
            
            OSTaskNameGet(subs[j].subscribed_os_pri, &task_name_ptr, &perr);
            OS_CHECK_PERR(perr, "TKVS Subscribers: OSTaskNameGet");
			bsp_termios_printf(port, "%16s", task_name_ptr);
			bsp_termios_printf(port, " - Src: %u Mask: %04x Hits: %u\r\n", subs[j].source, subs[j].event_mask, subs[j].hits);
        }
        else
        {
            break;
        }
    }
	// Note the +1 hackery - we are responding to a TKVS message so we have not freed it yet
	bsp_termios_printf(port, "Allocs: %u\r\nFree: %u\r\n", s_alloc_cnt, s_free_cnt+1);
}

static bsp_sti_command_t tkvs_command =
{
    .name = "tkvs",
    .handler = &tkvs_handler,
    .minArgs = 0,
    .maxArgs = 1,
    STI_HELP("tkvs [-v]                             Show TKVS statistics")
};

#endif // CONFIG_STI_CMD_TKVS

void bsp_tkvs_init(void)
{
	print_dbg("Initializing TKVS\r\n");
    s_alloc_cnt = 0;
    s_free_cnt = 0;
    memset(s_subscriptions, 0, sizeof(s_subscriptions));
    
    #ifdef CONFIG_BSP_TKVS_ENABLE_1STICK
    bsp_tkvs_init_timer(&tkvs_1s_tick, BSP_TKVS_TIMER_PERIODIC, 0, BSP_TKVS_SRC_CLOCK, BSP_CLOCK_EVENT_TICK1S, 
                        BSP_TKVS_TIMER_MS_TO_TICKS(1000));
    bsp_tkvs_start_timer(&tkvs_1s_tick);
    #endif
}

void bsp_tkvs_sti_init(void)
{
	#ifdef CONFIG_STI_CMD_TKVS
	bsp_sti_register_command(&tkvs_command);
	#endif
}

bsp_tkvs_msg_t* bsp_tkvs_alloc(uint16_t data_length)
{
    irqflags_t flags;
    
    bsp_tkvs_msg_t* msg = (bsp_tkvs_msg_t*) bsp_malloc(sizeof(bsp_tkvs_msg_t) + data_length);
    msg->int_hdr.ref = 0;
    flags = cpu_irq_save();
    s_alloc_cnt++;
    cpu_irq_restore(flags);
	//print_dbg("bsp_tkvs_alloc()="); print_dbg_int(data_length); print_dbg("\r\n");
    return msg;
}

void bsp_tkvs_free(bsp_tkvs_msg_t* msg)
{
    irqflags_t flags;
        
    if (msg->int_hdr.ref > 0 && --msg->int_hdr.ref == 0)
    {
        flags = cpu_irq_save();
        s_free_cnt++;
        cpu_irq_restore(flags);
		
		/*{		
			INT8U *task_name_ptr;
			INT8U perr;
			print_dbg("bsp_tkvs_free()=");
			OSTaskNameGet(OSPrioCur, &task_name_ptr, &perr);
			OS_CHECK_PERR(perr, "TKVS Subscribers: OSTaskNameGet");
			print_dbgn((const char *)task_name_ptr, 16);
			print_dbg("\r\n");
		}*/

        bsp_free(msg);
    }
}

bool bsp_tkvs_subscribe(uint8_t source, uint16_t event_mask, OS_EVENT* queue, INT8U task_prio)
{
    int i;
    
    if (queue && event_mask)
    {
        for (i=0; i<CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS; i++)
        {
            if (s_subscriptions[i].queue == 0)
            {
                s_subscriptions[i].source = source;
                s_subscriptions[i].event_mask = event_mask;
                s_subscriptions[i].queue = queue;
                s_subscriptions[i].subscribed_os_pri = task_prio;
                return true;
            }
        }
    }
    
    return false;
}

void bsp_tkvs_publish(uint8_t source, uint16_t event, bsp_tkvs_msg_t* msg)
{
	INT8U *task_name_ptr;
    subscription_t* s;
    irqflags_t flags;
    INT8U perr;
    
    msg->source = source;
    msg->event = event;
    
    // Lock the scheduler so that we queue up all the messages before allowing
    // another task to run
    if (OSIntNesting == 0) OSSchedLock();
    
    for (s = &s_subscriptions[0]; s->queue; s++)
    {
        if (source == s->source && (event & s->event_mask))
        {
            msg->int_hdr.ref++;
            s->hits++;
            perr = OSQPost(s->queue, msg);
            OS_CHECK_PERR(perr, "TKVS Publish: OSQPost");
			if (perr != OS_ERR_NONE)
			{
				OSTaskNameGet(s->subscribed_os_pri, &task_name_ptr, &perr);
				OS_CHECK_PERR(perr, "TKVS Subscribers: OSTaskNameGet");
				print_dbgn((const char *)task_name_ptr, 16);
				print_dbg_int(s->source);
				print_dbg("\r\n");
			}				
        }
    }
    
    if (msg->int_hdr.ref == 0)
    {
        // no matches	
        flags = cpu_irq_save();
        s_free_cnt++;
        cpu_irq_restore(flags);		
		/*{
	        INT8U *task_name_ptr;
	        INT8U perr;
	        print_dbg("bsp_tkvs_free()=");
	        OSTaskNameGet(OSPrioCur, &task_name_ptr, &perr);
	        OS_CHECK_PERR(perr, "TKVS Subscribers: OSTaskNameGet");
	        print_dbgn((const char *)task_name_ptr, 16);
	        print_dbg("\r\n");
        }*/
        bsp_free(msg);
    }
    
    // here we go
    if (OSIntNesting == 0) OSSchedUnlock();
}

void bsp_tkvs_publish_immed(uint8_t source, uint16_t event, uint8_t data)
{
    bsp_tkvs_msg_t* msg = bsp_tkvs_alloc(0);
	msg->data_len = 0;
    msg->immed_data = data;
    bsp_tkvs_publish(source,event,msg);
}

void bsp_tkvs_publish_data(uint8_t source, uint16_t event, const void* data, uint16_t data_length)
{
    bsp_tkvs_msg_t* msg = bsp_tkvs_alloc(data_length);
	msg->data_len = data_length;
    memcpy(msg->data, data, data_length);
    bsp_tkvs_publish(source,event,msg);
}

void bsp_tkvs_publish_immed_with_data(uint8_t source, uint16_t event, uint8_t immed_data, const void* data, uint16_t data_length)
{
    bsp_tkvs_msg_t* msg = bsp_tkvs_alloc(data_length);
    msg->data_len = data_length;
    memcpy(msg->data, data, data_length);
	msg->immed_data = immed_data;
    bsp_tkvs_publish(source,event,msg);	
}

bool bsp_tkvs_is_subscribed(uint8_t source, uint16_t event)
{
    subscription_t* s;
    for (s = &s_subscriptions[0]; s->queue; s++)
    {
        if (source == s->source && (event & s->event_mask))
        {
            return true;
        }
    }
    
    return false;
}

#else	// CONFIG_BSP_UCOS

#error "TKVS Requires UCOS"

#endif	// CONFIG_BSP_UCOS

#endif // CONFIG_BSP_ENABLE_TKVS