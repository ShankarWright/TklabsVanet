/**
 *	@file	fletcher.c
 *
 *	@brief	Fletcher's Checksum
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

uint16_t bsp_util_fletcher16(const uint8_t* data, int bytes)
{
	uint16_t sum1 = 0xff, sum2 = 0xff;
	
	while (bytes)
	{
		size_t tlen = bytes > 20 ? 20 : bytes;
		bytes -= tlen;
		do {
			sum2 += sum1 += *data++;
		} while (--tlen);
		sum1 = (sum1 & 0xff) + (sum1 >> 8);
		sum2 = (sum2 & 0xff) + (sum2 >> 8);
	}
	/* Second reduction step to reduce sums to 8 bits */
	sum1 = (sum1 & 0xff) + (sum1 >> 8);
	sum2 = (sum2 & 0xff) + (sum2 >> 8);
	return sum2 << 8 | sum1;
}

void bsp_util_running_xsum_init(bsp_running_fletcher* r)
{
	r->sum1 = 0xffff;
	r->sum2 = 0xffff;
	r->count = 21;
}

void bsp_util_running_xsum_addb(bsp_running_fletcher* r, uint8_t b)
{
	r->sum1 += b;
	r->sum2 += r->sum1;

	r->count--;         // every 21 bytes, do an extra calculation
	if (r->count == 0)
	{
		r->sum1 = (r->sum1 & 0xff) + (r->sum1 >> 8);
		r->sum2 = (r->sum2 & 0xff) + (r->sum2 >> 8);
		r->count = 21;
	}
}

void bsp_util_running_xsum_add(bsp_running_fletcher* r, const uint8_t* b, int len)
{
	while (len--) bsp_util_running_xsum_addb(r,*b++);
}

uint16_t bsp_util_running_xsum_result(bsp_running_fletcher* r)
{
	if (r->count!=21)
	{
		r->sum1 = (r->sum1 & 0xff) + (r->sum1 >> 8);
		r->sum2 = (r->sum2 & 0xff) + (r->sum2 >> 8);
	}
	// extra reduction step at the end
	r->sum1 = (r->sum1 & 0xff) + (r->sum1 >> 8);
	r->sum2 = (r->sum2 & 0xff) + (r->sum2 >> 8);
	return (r->sum2 << 8) | r->sum1;
}