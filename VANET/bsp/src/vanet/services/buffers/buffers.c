/**
 *	@file	buffers.c
 *
 *	@brief	Dynamic memory allocation
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
#include <stdarg.h>
#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_BUFFERS
#include "conf_buffers.h"

typedef struct  
{
    unsigned used       :1;
    unsigned pad        :7;
    uint8_t pool;
    uint16_t alloc_size;
    #ifdef CONFIG_BSP_BUFFERS_BOUNDARY_CHECK
    uint32_t magic;
    #endif
} buffer_header_t;

typedef struct  
{
    uint16_t bufsize;
    uint16_t count;
    uint8_t* pool;
    uint32_t allocs;
    uint32_t frees;
    int16_t max_used;
    uint16_t max_size;
} pool_t;

#ifdef CONFIG_BSP_BUFFERS_BOUNDARY_CHECK
#define BUFFER_OVERHEAD             12
#define BOUNDARY_MAGIC              0xdecafbad
static const uint32_t               s_magic = BOUNDARY_MAGIC;
#else
#define BUFFER_OVERHEAD             4
#endif

#define BUFFER_POOL_DEF(N,unused) \
    static uint8_t s_pool##N [(CONFIG_BSP_BUFFERS_POOL##N##_BUFSIZE + BUFFER_OVERHEAD) * CONFIG_BSP_BUFFERS_POOL##N##_COUNT] = { 0 };
MREPEAT(CONFIG_BSP_BUFFERS_NUM_POOLS,BUFFER_POOL_DEF,~)

#define POOL_DEF(N,unused) \
    { CONFIG_BSP_BUFFERS_POOL##N##_BUFSIZE, CONFIG_BSP_BUFFERS_POOL##N##_COUNT, s_pool##N, 0, 0, 0, 0 },
static pool_t s_pools[CONFIG_BSP_BUFFERS_NUM_POOLS] = {
    MREPEAT(CONFIG_BSP_BUFFERS_NUM_POOLS,POOL_DEF,~)
};

#ifdef CONFIG_STI_CMD_BUF
#undef UNIT_TESTS
static void buf_handler(int argc, char** argv, uint8_t port)
{
    #ifdef UNIT_TESTS
    if (argc > 2 && stricmp(argv[1],"test") == 0)
    {
        int n = strtoul(argv[2],0,10);
        if (n == 1)
        {
            for (uint16_t size=1; size < 1024; size++)
            {
                bsp_termios_printf(port, "malloc %d\r\n", size);
                uint8_t* buf = (uint8_t*) bsp_malloc(size);
                memset(buf,0xaa,size);
                bsp_free(buf);
                if ((size&0x7f) == 0) OSTimeDly(1);
            }
        }
        else if (n == 2)
        {
            for (int i=1; ; i++)
            {
                bsp_termios_printf("malloc %d\r\n", i);
                bsp_malloc(16);
                if ((i&0x7f) == 0) OSTimeDly(1);
            }
        }
        else if (n == 3)
        {
            bsp_malloc(0);
        }
        else if (n == 4)
        {
            uint8_t* buf = (uint8_t*) bsp_malloc(16);
            memset(buf,0xaa,17);
            bsp_free(buf);
        }
        else if (n == 5)
        {
            uint8_t* buf = (uint8_t*) bsp_malloc(16);
            memset(buf-1,0xaa,17);
            bsp_free(buf);
        }
        else if (n == 6)
        {
            uint8_t* buf = (uint8_t*) bsp_malloc(16);
            bsp_free(buf);
            bsp_free(buf);
        }
        else if (n == 7)
        {
            bsp_free((uint8_t*) 0x12345678);
        }
        else if (n == 8)
        {
            bsp_free(0);
        }
    }
    #endif
    
    bsp_buffer_dump(port);
}

static bsp_sti_command_t buf_command =
{
    .name = "buf",
    .handler = &buf_handler,
    .minArgs = 0,
    .maxArgs = 2,
    STI_HELP("buf                                    Show buffer statistics")
};
#endif

void bsp_buffers_init(void)
{
    // pools initialized to 0
    
    #ifdef CONFIG_STI_CMD_BUF
    bsp_sti_register_command(&buf_command);
    #endif
}

void* bsp_malloc(size_t bytes)
{
    void * buf = 0;
    irqflags_t flags;
    
    if (bytes == 0) bsp_reset(BSP_RESET_ALLOC_NULL);
    //print_dbg("malloc "); print_dbg_ulong(bytes); print_dbg("\r\n");

    flags = cpu_irq_save();
    for (int i=0; i<CONFIG_BSP_BUFFERS_NUM_POOLS && buf == 0; i++)
    {
        pool_t* pool = &s_pools[i];
        if (bytes <= pool->bufsize)
        {
            uint8_t* p = pool->pool;
            for (uint16_t j=0; j<pool->count; j++)
            {
                if (((buffer_header_t*)p)->used == 0)
                {
                    buffer_header_t* hdr = (buffer_header_t*)p;
                    hdr->used = 1;
                    hdr->pool = (uint8_t) i;
                    hdr->alloc_size = (uint16_t) bytes;
                    
                    #ifdef CONFIG_BSP_BUFFERS_BOUNDARY_CHECK
                    hdr->magic = BOUNDARY_MAGIC;
                    memcpy(p+8+bytes,&s_magic,4);
                    buf = p + 8;
                    #else
                    buf = p + 4;
                    #endif
                    
                    if (pool->max_size < bytes) pool->max_size = bytes;
                    if (pool->allocs < 0xffffffff)
                    {
                        pool->allocs++;
                    }
                    else
                    {
                        pool->allocs = 0;
                        pool->frees = 0;
                    }
                    if (pool->max_used < (pool->allocs - pool->frees))
                    {
                        pool->max_used = pool->allocs - pool->frees;
                    }
                    break;
                }
                p += pool->bufsize + BUFFER_OVERHEAD;
            }
        }
    }
    cpu_irq_restore(flags);
    
    if (buf == 0)
    {
        print_dbg("malloc failed "); print_dbg_int(bytes); print_dbg("\r\n");
        bsp_logcat_printf(BSP_LOGCAT_CRITICAL, "malloc failed %d", bytes);
        bsp_reset(BSP_RESET_ALLOC_FAIL);
    }
    return buf;
}

void bsp_free(void* ptr)
{
    uint8_t* p;
    buffer_header_t* hdr;
    pool_t* pool;
    irqflags_t flags;
    
    if (ptr == 0) bsp_reset(BSP_RESET_FREE_INVALID);
    
    p = (uint8_t*)((int)ptr & ~0x3); // 4-byte align
    #ifdef CONFIG_BSP_BUFFERS_BOUNDARY_CHECK
    hdr = (buffer_header_t*) (p - 8);
    #else
    hdr = (buffer_header_t*) (p - 4);
    #endif
    
    if (hdr->pool >= CONFIG_BSP_BUFFERS_NUM_POOLS || hdr->used != 1) bsp_reset(BSP_RESET_FREE_INVALID);
    pool = &s_pools[hdr->pool];
    if (p < pool->pool || p >= (pool->pool + (pool->bufsize+BUFFER_OVERHEAD)*pool->count)) bsp_reset(BSP_RESET_FREE_INVALID);
    
    #ifdef CONFIG_BSP_BUFFERS_BOUNDARY_CHECK
    if (hdr->magic != BOUNDARY_MAGIC || memcmp(p+hdr->alloc_size,&s_magic,4))
    {
        bsp_reset(BSP_RESET_FREE_BOUNDARY);
    }
    #endif
    
    flags = cpu_irq_save();
    hdr->used = 0;
    pool->frees++;
    cpu_irq_restore(flags);
}

void bsp_buffer_dump(uint8_t port)
{
    for (int i=0; i<CONFIG_BSP_BUFFERS_NUM_POOLS; i++)
    {
        bsp_termios_printf(port, "Pool %d (%db x %d)\r\nAllocs: %d\r\nUsed: %d\r\nMax Used: %d\r\nMax Size: %d\r\n\r\n",
            i, s_pools[i].bufsize, s_pools[i].count, s_pools[i].allocs, s_pools[i].allocs - s_pools[i].frees,
            s_pools[i].max_used, s_pools[i].max_size);
    }
}

#endif