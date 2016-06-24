/**
 *	@file	str_utils.c
 *
 *	@brief	String Utilities
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

#include <string.h>
#include <asf.h>
#include "vanet.h"

#define WHITESPACE          " \t\r\n"

uint8_t bsp_util_convert_hex(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	else return 0;
}

int bsp_util_decode_hex_string(const char* str, uint8_t* data, int data_length)
{
	int bytes = 0;
	while (str[0] && str[1] && data_length)
	{
		*data++ = (0xf0 & (bsp_util_convert_hex(str[0]) << 4)) | (0x0f & bsp_util_convert_hex(str[1]));
		str += 2;
		--data_length;
		++bytes;
	}
	return bytes;
}

void bsp_util_int_to_str(char* str, int n)
{
	int div = 100000000;
	if (n < 0) *str++ = '-';
	while (n / div == 0) div /= 10;
	while (div > 0)
	{
		*str++ = (n / div) + '0';
		n = n % div;
		div /= 10;
	}
	*str = 0;
}

void bsp_util_hex_to_str(char* str, uint32_t n)
{
	bsp_util_short_hex_to_str(str, (uint16_t)((n >> 16) & 0xffff));
	bsp_util_short_hex_to_str(str+4, (uint16_t)(n & 0xffff));
}

void bsp_util_short_hex_to_str(char* str, uint16_t n)
{
	bsp_util_char_hex_to_str(str, (uint8_t)((n >> 8) & 0xff));
	bsp_util_char_hex_to_str(str+2, (uint8_t)(n & 0xff));
}

static inline char nibble_to_char(uint8_t n)
{
	return n > 9 ? 'A' + n - 10 : '0' + n;
}

void bsp_util_char_hex_to_str(char* str, uint8_t n)
{
	*str++ = nibble_to_char((n >> 4) & 0xf);
	*str++ = nibble_to_char(n & 0xf);
	*str = 0;
}

char* bsp_util_strtrim(char* str)
{
    char* start, *end;
    
    while (*str && strchr(WHITESPACE, *str)) str++;
    start = str;
    
    end = 0;
    while (*str)
    {
        if (strchr(WHITESPACE, *str))
        {
            if (end == 0) end = str;
        }
        else
        {
            end = 0;
        }
        str++;
    }
    if (end) *end = 0;
    
    return start;
}

int bsp_util_strsplit(char* source, const char* delimiters, char** strs, int max_strs)
{
    int n = 0;

    strs[n++] = source;
    while (*source)
    {
        if (strchr(delimiters, *source))
        {
            *source = 0;
            if (n < max_strs)
            {
                strs[n++] = source+1;
            }
            else
            {
                break;
            }
        }
        source++;
    }
    return n;
}
