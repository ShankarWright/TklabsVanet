/**
 *	@file	str_utils.h
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

#ifndef STR_UTILS_H
#define STR_UTILS_H

/// Convert an ascii character into its hexadecimal value
extern uint8_t bsp_util_convert_hex(char c);

/**
 *  Decode an ascii string of hexadecimal digits into a byte array
 *
 *  @return The number of bytes decoded
 */
extern int bsp_util_decode_hex_string(const char* str, uint8_t* data, int max_bytes);

/// Convert a number to a string
extern void bsp_util_int_to_str(char* str, int n);

/// Convert a number to a string
extern void bsp_util_hex_to_str(char* str, uint32_t n);

/// Convert a number to a string
extern void bsp_util_short_hex_to_str(char* str, uint16_t n);

/// Convert a number to a string
extern void bsp_util_char_hex_to_str(char* str, uint8_t n);

/// remove leading and trailing whitespace. return ptr to trimmed string
extern char* bsp_util_strtrim(char* str);

// split a string
extern int bsp_util_strsplit(char* source, const char* delimiters, char** strs, int max_strs);


#endif // STR_UTILS_H