/**
 *	@file	print_funcs_ext.c
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

#include "print_funcs_ext.h"

#define IS_PRINTABLE(x)         (((x) >= 0x20) && ((x) <= 0x7E))

void print_array(volatile avr32_usart_t *usart, const uint8_t* addr, int len)
{
    char ascii[17];
    int i = 0;
    
    ascii[16] = 0;
    while (len > 0)
    {
        char c = *addr++;
        print_char_hex(usart,c);
        print_char(usart,' ');
        
        ascii[i] = IS_PRINTABLE(c) ? c : '.';
        if (++i == 16)
        {
            print(usart,"   ");
            print(usart,ascii);
            print(usart,"\r\n");
            i = 0;
        }          
        --len;
    }
    if (i > 0)
    {
        while (i < 16)
        {
            print(usart,"   ");
            ascii[i] = ' ';
            i++;
        }
        print(usart,"   ");
        print(usart,ascii);
        print(usart,"\r\n");
    }     
}

void print_array_plain(volatile avr32_usart_t *usart, const uint8_t* addr, int len)
{
    while (len > 0)
    {
        print_char_hex(usart,*addr++);
        --len;
    }
}

void print_int(volatile avr32_usart_t *usart, int x)
{
    if (x < 0)
    {
        print_char(usart,'-');
        x = -x;
    }
    print_ulong(usart,x);
}

void print_intk(volatile avr32_usart_t *usart, int x)
{
    int whole = x / 1000;
    int frac = abs(x) % 1000;
               
    if (whole == 0 && x < 0)
    {
        print(usart,"-0");
    }        
    else
    {
        print_int(usart,whole);
    }        
    
    print_char(usart,'.');
    
    if (frac < 100) print_char(usart,'0');
    if (frac < 10) print_char(usart,'0');
    print_ulong(usart,frac);
}

void printn(volatile avr32_usart_t *usart, const char* str, uint32_t n)
{
    while (n > 0)
    {
        print_char(usart,*str ? *str++ : ' ');
        n--;
    }
}
