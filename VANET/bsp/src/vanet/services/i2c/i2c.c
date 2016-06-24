/**
 *	@file	i2c.c
 *
 *	@brief	I2C Driver
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

#include <asf.h>
#include "vanet.h"
#include <string.h>

#ifdef CONFIG_BSP_ENABLE_I2C

static bool s_twim_inititialized = false;

#ifdef CONFIG_BSP_UCOS
static OS_EVENT *s_i2c_mutex;
#endif // CONFIG_BSP_UCOS

const gpio_map_t TWIM_GPIO_MAP = {
{AVR32_TWIMS0_TWCK_0_0_PIN, AVR32_TWIMS0_TWCK_0_0_FUNCTION},
{AVR32_TWIMS0_TWD_0_0_PIN, AVR32_TWIMS0_TWD_0_0_FUNCTION}
};

twi_options_t TWIM_OPTIONS = {
	.pba_hz = 0,					/* set @ runtime */
	.speed = 50000,
	.chip = 0,						/* this can be ignored - init() probes this address? */
	.smbus = false,
};

#ifdef CONFIG_STI_CMD_I2C
static void i2c_handler(int argc, char** argv, uint8_t port)
{
	bsp_termios_write_str(port, "Slaves found: ");
	for (int i=1; i<127; i++)
	{
		status_code_t status = twim_probe(&AVR32_TWIM0, i);
		if (status == STATUS_OK)
		{
			bsp_termios_printf(port, "%02X ", i);
		}
		else
		{
			bsp_termios_write_str(port, " - ");
		}
	}
	bsp_termios_write_str(port, "\r\n");
}

bsp_sti_command_t i2c_command =
{
	.name = "i2c",
	.handler = &i2c_handler,
	.minArgs = 0,
	.maxArgs = 0
};
#endif // CONFIG_STI_CMD_I2C

/************************************************************************/
/* I2C Service Arbitration                                              */
/************************************************************************/
static inline void i2c_lock(void)
{
#ifdef CONFIG_BSP_UCOS
	uint8_t perr;
	OSMutexPend(s_i2c_mutex, 0, &perr);
#endif // CONFIG_BSP_UCOS
}

static inline void i2c_unlock(void)
{
#ifdef CONFIG_BSP_UCOS
	OSMutexPost(s_i2c_mutex);
#endif // CONFIG_BSP_UCOS
}

void bsp_i2c_init()
{
#ifdef CONFIG_BSP_UCOS
	INT8U perr;
#endif // CONFIG_BSP_UCOS
	
	print_dbg("Initializing I2C\r\n");
	if (!s_twim_inititialized)
	{	
		gpio_enable_module (TWIM_GPIO_MAP,
			sizeof (TWIM_GPIO_MAP) / sizeof (TWIM_GPIO_MAP[0]));
		
		TWIM_OPTIONS.pba_hz = sysclk_get_pba_hz();
		twim_master_init (&AVR32_TWIM0, &TWIM_OPTIONS);
	
#ifdef CONFIG_BSP_UCOS
		s_i2c_mutex = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &perr);
		OS_CHECK_PERR(perr, "bsp_i2c_init: OSMutexCreate");
#endif // CONFIG_BSP_UCOS
		
		s_twim_inititialized = true;
		
		#ifdef CONFIG_STI_CMD_I2C
		bsp_sti_register_command(&i2c_command);
		#endif
	}
}

/************************************************************************/
/* Generic I2C Access                                                   */
/************************************************************************/
status_code_t bsp_i2c_read_bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *bytes, uint8_t len)
{
	status_code_t status;
	uint8_t i2c_data[16];
	twim_package_t i2c_pkt = {
		.chip = 0,
		.addr[0] = 0,
		.addr_length = 1,
		.buffer = i2c_data,
		.length = 0,
	};
	
	if (len > 16)
		return ERR_INVALID_ARG;
		
	i2c_pkt.chip = i2c_addr;
	i2c_pkt.addr[0] = dev_addr;
	i2c_pkt.length = len;
	i2c_lock();
	status = twim_read_packet(&AVR32_TWIM0, &i2c_pkt);
	i2c_unlock();
	
	for (int i=0; i<len; i++)
		bytes[i] = i2c_data[i];
	
	return status;
}

status_code_t bsp_i2c_read_byte(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *value)
{
	status_code_t status;
	uint8_t bytes;

	status = bsp_i2c_read_bytes(i2c_addr, dev_addr, &bytes, 1);
	*value = bytes;
	
	return status;
}

status_code_t bsp_i2c_read_2bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *value1, uint8_t *value2)
{
	status_code_t status;
	uint8_t bytes[2];
		
	status = bsp_i2c_read_bytes(i2c_addr, dev_addr, bytes, 2);
	*value1 = bytes[0];
	*value2 = bytes[1];
		
	return status;
}

status_code_t bsp_i2c_write_byte_raw(uint8_t i2c_addr, uint8_t value)
{
	status_code_t status;
	uint8_t data[1] = {value};
	
	i2c_lock();
	status = twim_write(&AVR32_TWIM0, data, 1, i2c_addr, false);
	i2c_unlock();
	
	return status;
}

status_code_t bsp_i2c_write_bytes(uint8_t i2c_addr, uint8_t dev_addr, uint8_t *bytes, uint8_t len)
{
	status_code_t status;
	uint8_t i2c_data[16];
	twim_package_t i2c_pkt = {
		.chip = 0,
		.addr[0] = 0,
		.addr_length = 1,
		.buffer = i2c_data,
		.length = 0,
	};
	
	if (len > 16)
		return ERR_INVALID_ARG;
	
	// set up write packet
	i2c_pkt.chip = i2c_addr;
	i2c_pkt.addr[0] = dev_addr;
	i2c_pkt.length = len;
	for (int i=0; i<len; i++)
		i2c_data[i] = bytes[i];
		
	i2c_lock();
	status = twim_write_packet(&AVR32_TWIM0, &i2c_pkt);
	i2c_unlock();
	
	
	return status;
}


status_code_t bsp_i2c_write_byte_verbose(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value)
{
	status_code_t status;
	uint8_t check_val;

	bsp_i2c_read_bytes(i2c_addr, dev_addr, &check_val, 1);
	print_dbg("before = ");
	print_dbg_hex(check_val);
	print_dbg("\r\n");

	status = bsp_i2c_write_bytes(i2c_addr, dev_addr, &value, 1);
	
	bsp_i2c_read_bytes(i2c_addr, dev_addr, &check_val, 1);
	print_dbg("after = ");
	print_dbg_hex(check_val);
	print_dbg("\r\n");
	
	return status;
}

status_code_t bsp_i2c_write_byte(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value)
{
	status_code_t status;
	
	status = bsp_i2c_write_bytes(i2c_addr, dev_addr, &value, 1);
	
	return status;
}

status_code_t bsp_i2c_write_2bytes_verbose(uint8_t i2c_addr, uint8_t dev_addr, uint8_t value1, uint8_t value2)
{
	status_code_t status;
	uint8_t bytes[2], check_val[2];
	
	// before
	bsp_i2c_read_bytes(i2c_addr, dev_addr, check_val, 2);
	print_dbg("before = ");
	print_dbg_hex(check_val[0]); print_dbg(" "); print_dbg_hex(check_val[1]);
	print_dbg("\r\n");
	
	// write packet
	bytes[0] = value1;
	bytes[1] = value2;
	status = bsp_i2c_write_bytes(i2c_addr, dev_addr, bytes, 2);
	
	// after
	bsp_i2c_read_bytes(i2c_addr, dev_addr, check_val, 2);
	print_dbg("after = ");
	print_dbg_hex(check_val[0]); print_dbg(" "); print_dbg_hex(check_val[1]);
	print_dbg("\r\n");
	
	return status;
}

#endif // CONFIG_BSP_ENABLE_I2C


/************************************************************************/
/* Debuggery                                                            */
/************************************************************************/
#ifdef BSP_I2C_TRACE
void bsp_i2c_trace(const char *s)
{
	print_dbg(s);
	print_dbg("\r\n");
}

void bsp_i2c_trace_ul(const char *s, unsigned long p)
{
	print_dbg(s);
	print_dbg(" ");
	print_dbg_int(p);
	print_dbg("\r\n");
}

void bsp_i2c_trace_hex(const char *s, unsigned long p)
{
	print_dbg(s);
	print_dbg(" ");
	print_dbg_hex(p);
	print_dbg("\r\n");
}

void bsp_i2c_trace_2(const char *s, unsigned long p1, unsigned long p2)
{
	print_dbg(s);
	print_dbg(" ");
	print_dbg_int(p1);
	print_dbg(" ");
	print_dbg_int(p2);
	print_dbg("\r\n");
}
#endif // BSP_I2C_TRACE