/**
 *	@file	circ.h
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

#ifndef _CIRC_H
#define _CIRC_H

/**
 * @defgroup group_bsp_utils_circ Circular buffer
 *
 * Circular buffer implementation
 *
 * @{
 */

#ifndef UNIT_TEST
#include <avr32/io.h>
#include "compiler.h"
#endif

/// Circular buffer data
typedef struct
{
    uint8_t* buffer;
    uint16_t length;
    volatile uint16_t read;
    volatile uint16_t write;
    volatile uint16_t size;
} bsp_circ_buffer_t;

/** @name Circular Buffer
 *
 *  A interrupt-safe circular buffer
 *
 *  @{
 */

/// Initialize a circular buffer
extern void bsp_circ_init(bsp_circ_buffer_t* c, void* buffer, uint16_t buffer_length);

/** 
 * Write data to the queue
 *
 * @return      number of bytes written
 */
extern uint16_t bsp_circ_write(bsp_circ_buffer_t* c, const void* data, uint16_t data_length);

/** 
 * Write data to the queue
 *
 * @return      number of bytes written
 */
extern uint16_t bsp_circ_writeb(bsp_circ_buffer_t* c, uint8_t data);

/**
 * Read data from the queue.  Read bytes are removed from the queue
 * 
 * @return      number of bytes read
 */
extern uint16_t bsp_circ_read(bsp_circ_buffer_t* c, void* data, uint16_t data_length);

/**
 * Read a byte from the queue.  Read byte is removed from the queue
 *
 * @return      the byte read
 */
extern int16_t bsp_circ_readb(bsp_circ_buffer_t* c);

/// A queue iterator
typedef uint16_t bsp_circ_iter_t;

/// Create an iterator at the beginning of the queue
static inline bsp_circ_iter_t bsp_circ_begin(bsp_circ_buffer_t* c)
{
    return c->read;
}

/// Advance an iterator
extern bsp_circ_iter_t bsp_circ_adv(bsp_circ_buffer_t* c, bsp_circ_iter_t i, uint16_t size);

/// Test if an iterator is at the end of the queue
static inline bool bsp_circ_eof(bsp_circ_buffer_t* c, bsp_circ_iter_t i)
{
    return i == c->write;
}

/**
 * Read data from the queue at a position.  Read bytes are not removed from the queue
 * 
 * @return      number of bytes read
 */
extern uint16_t bsp_circ_peek(bsp_circ_buffer_t* c, bsp_circ_iter_t i, void* data, uint16_t data_length);

/**
 * Read a byte from the queue at a position.  Byte is not removed from the queue
 * 
 * @return      byte read
 */
extern uint8_t bsp_circ_peekb(bsp_circ_buffer_t* c, bsp_circ_iter_t i);

/**
 * Determine how many bytes available to read from a position
 * 
 * @return      byte read
 */
extern uint16_t bsp_circ_available(bsp_circ_buffer_t* c, bsp_circ_iter_t i);

/// Test if the buffer is full
#define bsp_circ_full(c)         ((c)->size == (c)->length - 1)

/// Get the number of bytes used in the buffer
#define bsp_circ_size(c)         ((c)->size)

/// Get the number of bytes available in the buffer
#define bsp_circ_free(c)         ((c)->length - 1 - (c)->size)

/// Get the length of the buffer
#define bsp_circ_length(c)		 ((c)->length - 1)

/// Clear the log
extern void bsp_circ_clear(bsp_circ_buffer_t* c);

/// @}

/// @}

#endif  // _CIRC_H
