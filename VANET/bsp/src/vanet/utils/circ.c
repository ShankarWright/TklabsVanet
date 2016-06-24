/**
 *	@file	circ.c
 *
 *	@brief	Circular Buffer
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

#ifdef UNIT_TEST
typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#define min(a,b)    ((a) < (b) ? (a) : (b))
#endif

#include <string.h>
#include "circ.h"

#ifdef UNIT_TEST
#define circ_lock()
#define circ_unlock()
#else
#define circ_lock()         { irqflags_t flags = cpu_irq_save();
#define circ_unlock()       cpu_irq_restore(flags); }
#endif

void bsp_circ_init(bsp_circ_buffer_t* c, void* buffer, uint16_t buffer_length)
{
    c->buffer = (uint8_t*)buffer;
    c->length = buffer_length;
    c->read = 0;
    c->write = 0;
    c->size = 0;
}

uint16_t bsp_circ_write(bsp_circ_buffer_t* c, const void* data, uint16_t data_length)
{
    uint16_t bytes_written = 0, bytes_to_write;
    const uint8_t* _data = data;
    circ_lock();
    while (data_length > 0)
    {
        if (bsp_circ_full(c))
        {
            break;
        }
        else if (c->write >= c->read)
        {
            bytes_to_write = min(data_length,c->length - c->write);
        }
        else
        {
            bytes_to_write = min(data_length,c->read - c->write - 1);
        }
                
        memcpy(c->buffer + c->write, _data, bytes_to_write);
        
        bytes_written += bytes_to_write;
        data_length -= bytes_to_write;
        _data += bytes_to_write;
        c->write += bytes_to_write;
        c->size += bytes_to_write;
        if (c->write == c->length) c->write = 0;
    }
    circ_unlock();
    return bytes_written;
}

uint16_t bsp_circ_writeb(bsp_circ_buffer_t* c, uint8_t data)
{
    uint16_t rc = 0;
    circ_lock();
    if (!bsp_circ_full(c))
    {
        c->buffer[c->write] = data;
        if (++c->write == c->length) c->write = 0;
        c->size++;
        rc = 1;
    }
    circ_unlock();
    return rc;
}

static uint16_t size_from_pos(bsp_circ_buffer_t* c, uint16_t pos)
{
    if (c->write == c->read)
    {
        return 0;
    }
    if (c->write > c->read)
    {
        if (pos >= c->read && pos < c->write)
        {
            return c->write - pos;
        }
        return 0;
    }
    if (pos < c->write)
    {
        return c->write - pos;
    }
    if (pos >= c->read && pos < c->length)
    {
        return c->length - pos + c->write;
    }
    return 0;
}

static uint16_t peek(bsp_circ_buffer_t* c, uint16_t pos, void* data, uint16_t data_length)
{
    uint16_t free = size_from_pos(c,pos);
    if (data_length > free) data_length = free;
    uint16_t bytes_read = data_length, bytes_to_read;
    uint8_t* _data = data;

    if (data)
    {
        while (data_length > 0)
        {
            bytes_to_read = min(data_length,c->length - pos);
            
            memcpy(_data, c->buffer + pos, bytes_to_read);
            
            data_length -= bytes_to_read;
            _data += bytes_to_read;
            pos += bytes_to_read;
            if (pos == c->length) pos = 0;
        }
    }
    
    return bytes_read;
}

uint16_t bsp_circ_read(bsp_circ_buffer_t* c, void* data, uint16_t data_length)
{
    circ_lock();
    
    // read the bytes
    data_length = peek(c,c->read,data,data_length);
    
    // advance the read pointer
    uint16_t pos = c->read + data_length;
    if (c->length - c->read <= data_length) pos -= c->length;
    c->read = pos;
    c->size -= data_length;
    
    circ_unlock();
    
    return data_length;
}

int16_t bsp_circ_readb(bsp_circ_buffer_t* c)
{
    int16_t b = -1;
    circ_lock();
    if (bsp_circ_size(c))
    {
        b = c->buffer[c->read];
        if (++c->read == c->length) c->read = 0;
        c->size--;
    }
    circ_unlock();
    return b;
}

bsp_circ_iter_t bsp_circ_adv(bsp_circ_buffer_t* c, bsp_circ_iter_t i, uint16_t size)
{
    uint16_t free = size_from_pos(c,i);
    if (size > free) size = free;
    
    i += size;
    if (i >= c->length) i -= c->length;
    return i;        
}

uint16_t bsp_circ_peek(bsp_circ_buffer_t* c, bsp_circ_iter_t pos, void* data, uint16_t data_length)
{
    circ_lock();
    data_length = peek(c,pos,data,data_length);
    circ_unlock();
    return data_length;
}

uint8_t bsp_circ_peekb(bsp_circ_buffer_t* c, bsp_circ_iter_t pos)
{
    uint8_t b = 0;
    circ_lock();
    if (size_from_pos(c,pos))
    {
        b = c->buffer[pos];
    }
    circ_unlock();
    return b;
}

uint16_t bsp_circ_available(bsp_circ_buffer_t* c, bsp_circ_iter_t i)
{
    uint16_t len;
    circ_lock();
    len = size_from_pos(c,i);
    circ_unlock();
    return len;
}

void bsp_circ_clear(bsp_circ_buffer_t* c)
{
    c->read = 0;
    c->write = 0;
    c->size = 0;
}

#ifdef UNIT_TEST
#include <stdlib.h>

static void printq(bsp_circ_buffer_t* q)
{
    printf("queue: l:%d s:%d r:%d w:%d\n", q->length, q->size, q->read, q->write);
}

int main(void)
{
    uint8_t buffer[17];
    bsp_circ_buffer_t q;
    uint8_t test[16];
    uint8_t c;
    bsp_circ_iter_t i;
        
    bsp_circ_init(&q,buffer,17);
    
    // test the byte-oriented methods
    for (c=0; c<8; c++) bsp_circ_writeb(&q,c);
    if (bsp_circ_size(&q) != 8) printf("bsp_circ_size wrong (1)\n");
    if (bsp_circ_full(&q)) printf("bsp_circ_full wrong (1)\n");
    if (bsp_circ_free(&q) != 8) printf("bsp_circ_free wrong (1)\n");
    for (c=0,i=bsp_circ_begin(&q); c<8; c++, i=bsp_circ_adv(&q,i,1))
    {
        if (bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (1)\n");
        if (bsp_circ_peekb(&q,i) != c) printf("bsp_circ_peekb wrong at %d (1)\n", c);
    }
    if (!bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (1.1)\n");
    for (c=0; c<8; c++)
    {
        if (bsp_circ_readb(&q) != c) printf("bsp_circ_readb wrong at %d (1)\n", c);
    }        
    for (c=0; c<16; c++) bsp_circ_writeb(&q,c);
    if (bsp_circ_size(&q) != 16) printf("bsp_circ_size wrong (2)\n");
    if (!bsp_circ_full(&q)) printf("bsp_circ_full wrong (2)\n");
    if (bsp_circ_free(&q) != 0) printf("bsp_circ_free wrong (2)\n");
    for (c=0,i=bsp_circ_begin(&q); c<16; c++, i=bsp_circ_adv(&q,i,1))
    {
        if (bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (2)\n");
        if (bsp_circ_peekb(&q,i) != c) printf("bsp_circ_peekb wrong at %d (2)\n", c);
    }
    if (!bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (2.1)\n");
    for (c=0; c<16; c++)
    {
        if (bsp_circ_readb(&q) != c) printf("bsp_circ_readb wrong at %d (2)\n", c);
    }
    
    // test block oriented
    for (c=0; c<8; c++) test[c] = c;
    if (bsp_circ_write(&q,test,8) != 8) printf("bsp_circ_write wrong (3)\n");
    if (bsp_circ_size(&q) != 8) printf("bsp_circ_size wrong (3)\n");
    if (bsp_circ_full(&q)) printf("bsp_circ_full wrong (3)\n");
    if (bsp_circ_free(&q) != 8) printf("bsp_circ_free wrong (3)\n");
    memset(test,0,16);
    if (bsp_circ_peek(&q,bsp_circ_begin(&q),test,4) != 4) printf("bsp_circ_peek wrong (3)\n");
    for (c=0; c<4; c++)
    {
        if (test[c] != c) printf("bsp_circ_peek wrong at %d (3)\n", c);
    }
    memset(test,0,16);
    i = bsp_circ_begin(&q);
    i = bsp_circ_adv(&q,i,4);
    if (bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (3.1)\n");
    if (bsp_circ_peek(&q,i,test,4) != 4) printf("bsp_circ_peek wrong (3.1)\n");
    for (c=0; c<4; c++)
    {
        if (test[c] != c+4) printf("bsp_circ_peek wrong at %d (3.1)\n", c);
    }
    memset(test,0,16);
    if (bsp_circ_read(&q,test,8) != 8) printf("bsp_circ_read wrong (3)\n");
    for (c=0; c<8; c++)
    {
        if (test[c] != c) printf("bsp_circ_read wrong at %d (3)\n", c);
    }
    for (c=0; c<16; c++) test[c] = c;
    if (bsp_circ_write(&q,test,16) != 16) printf("bsp_circ_write wrong (4)\n");
    if (bsp_circ_size(&q) != 16) printf("bsp_circ_size wrong (4)\n");
    if (!bsp_circ_full(&q)) printf("bsp_circ_full wrong (4)\n");
    if (bsp_circ_free(&q) != 0) printf("bsp_circ_free wrong (4)\n");
    memset(test,0,16);
    if (bsp_circ_peek(&q,bsp_circ_begin(&q),test,9) != 9) printf("bsp_circ_peek wrong (4)\n");
    for (c=0; c<9; c++)
    {
        if (test[c] != c) printf("bsp_circ_peek wrong at %d (4)\n", c);
    }
    memset(test,0,16);
    i = bsp_circ_begin(&q);
    i = bsp_circ_adv(&q,i,9);
    if (bsp_circ_eof(&q,i)) printf("bsp_circ_eof wrong (4.1)\n");
    if (bsp_circ_peek(&q,i,test,16) != 7) printf("bsp_circ_peek wrong (4.1)\n");
    for (c=0; c<7; c++)
    {
        if (test[c] != c+9) printf("bsp_circ_peek wrong at %d (4.1)\n", c);
    }
    memset(test,0,16);
    if (bsp_circ_read(&q,test,16) != 16) printf("bsp_circ_read wrong (4.1)\n");
    for (c=0; c<16; c++)
    {
        if (test[c] != c) printf("bsp_circ_read wrong at %d (4.1)\n", c);
    }
    
    printf("Done\n");
    return 0;
}

#endif // UNIT_TEST