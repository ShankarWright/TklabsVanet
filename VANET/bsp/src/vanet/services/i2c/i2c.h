/**
 *	@file	i2c.h
 *
 *	@brief	I2C Service
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

#ifndef _I2C_H
#define _I2C_H

#include <avr32/io.h>
#include "compiler.h"

/************************************************************************/
/* debug macros                                                         */
/************************************************************************/
#undef BSP_I2C_TRACE
#ifdef BSP_I2C_TRACE
extern void bsp_i2c_trace(const char *s);
extern void bsp_i2c_trace_ul(const char *s, unsigned long p);
extern void bsp_i2c_trace_hex(const char *s, unsigned long p);
extern void bsp_i2c_trace_2(const char *s, unsigned long p1, unsigned long p2);
#else // BSP_I2C_TRACE
#define bsp_i2c_trace(s)
#define bsp_i2c_trace_ul(s, p)
#define bsp_i2c_trace_hex(s, p)
#define bsp_i2c_trace_2(s, p1, p2)
#endif // BSP_I2C_TRACE

/// Initialize I2C
extern void bsp_i2c_init(void);


/************************************************************************/
/* Generic I2C Read/Write Byte                                          */
/************************************************************************/
status_code_t bsp_i2c_write_bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *bytes, uint8_t len);
status_code_t bsp_i2c_write_byte(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value);
status_code_t bsp_i2c_write_byte_verbose(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value);
status_code_t bsp_i2c_write_2bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value1, uint8_t value2);
status_code_t bsp_i2c_write_2bytes_verbose(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value1, uint8_t value2);

status_code_t bsp_i2c_read_bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *bytes, uint8_t len);
status_code_t bsp_i2c_read_byte(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *value);
status_code_t bsp_i2c_read_2bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *value1, uint8_t *value2);

status_code_t bsp_i2c_write_byte_raw(uint8_t i2c_addr, uint8_t value);
#endif  // _I2C_H
