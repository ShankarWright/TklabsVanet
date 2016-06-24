/**
 *	@file	print_funcs_ext.h
 *
 *	@brief	Extensions to Atmels print_ debug funtions
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

#ifndef _PRINT_FUNCS_EXT_H_
#define _PRINT_FUNCS_EXT_H_

#include <asf.h>

/// Print a byte array using hex digits with ascii decoding and wrapping at 16 bytes
extern void print_array(volatile avr32_usart_t *usart, const uint8_t* x, int bytes);

/// Print a byte array using hex digits
extern void print_array_plain(volatile avr32_usart_t *usart, const uint8_t* x, int bytes);

/// Print a byte array using hex digits with ascii decoding and wrapping at 16 bytes
static inline void print_dbg_array(const uint8_t* x, int bytes)
{
    print_array(DBG_USART,x,bytes);
}

/// Print a byte array using hex digits
static inline void print_dbg_array_plain(const uint8_t* x, int bytes)
{
    print_array_plain(DBG_USART,x,bytes);
}

/// Print a signed integer
extern void print_int(volatile avr32_usart_t *usart, int x);

/// Print a signed integer
static inline void print_dbg_int(int x)
{
    print_int(DBG_USART,x);
}

/// Print an integer divided by 1000.  Ex: 1034 -> 1.034
extern void print_intk(volatile avr32_usart_t *usart, int x);

/// Print an integer divided by 1000.  Ex: 1034 -> 1.034
static inline void print_dbg_intk(int x)
{
    print_intk(DBG_USART,x);
}

/// Print a string with exactly n chars.  if string is less than n chars, is filled in with spaces
extern void printn(volatile avr32_usart_t *usart, const char* str, uint32_t n);

/// Print a string with exactly n chars.  if string is less than n chars, is filled in with spaces
static inline void print_dbgn(const char* str, uint32_t n)
{
    printn(DBG_USART,str,n);
}


#endif
