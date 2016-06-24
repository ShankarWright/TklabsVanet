/**
 *	@file	accel_lis3dsh.c
 *
 *	@brief	ST LIS3D(S)H Accelerometer Handler
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
#include <string.h>
#include "vanet.h"
#include "vanet_api.h"

#ifdef BSP_ENABLE_LIS3DSH

#define LIS3DSH_OUT_T		0x0c
#define LIS3DSH_INFO1		0x0d
#define LIS3DSH_INFO2		0x0e
#define LIS3DSH_WHO_AM_I	0x0f
#define LIS3DSH_STAT		0x18
#define LIS3DSH_CTRL_REG4	0x20
#define LIS3DSH_CTRL_REG1	0x21
#define LIS3DSH_CTRL_REG2	0x22
#define LIS3DSH_CTRL_REG3	0x23
#define LIS3DSH_CTRL_REG5	0x24
#define LIS3DSH_STATUS		0x27
#define LIS3DSH_OUT_X		0x28
#define LIS3DSH_OUT_Y		0x2A
#define LIS3DSH_OUT_Z		0x2C

#define LIS3DSH_ST1_1		0x40
#define LIS3DSH_ST1_2		0x41

#define LIS3DSH_THRS1_1		0x57
#define LIS3DSH_MASK1_B		0x59
#define LIS3DSH_MASK1_A		0x5a
#define LIS3DSH_SETT1		0x5b
#define LIS3DSH_PR1			0x5c
#define LIS3DSH_OUTS1		0x5f


#define LIS3DSH_WHO_AM_I_EXPECTED	0x3f
#define LIS3DSH_INFO1_EXPECTED		0x21

#define LIS3DSG_1G_AT_2G_SCALE		0x4000

/*
typedef struct
{
	int16_t x_accel;
	int16_t y_accel;
	int16_t z_accel;
} lis3dsh_accel_t;
*/

bool app_accel_init_dev(void)
{
	uint8_t info1, who_am_i;
	
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_INFO1, &info1);
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_WHO_AM_I, &who_am_i);
	
	if ((who_am_i == LIS3DSH_WHO_AM_I_EXPECTED) &&
		(info1 == LIS3DSH_INFO1_EXPECTED))
	{
		bsp_logcat_print(BSP_LOGCAT_ACCEL, "LIS3DSH Accelerometer Found");
		
		// disable state machine!
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_THRS1_1, 0x00);		// threshold
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG3, 0x00);	// Interrupts disabled
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG1, 0x00);	// disable SM1
				
		// enable x,y,z @ 100Hz, +/- 2G scale
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG3, 0);		// disable interrupts
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG5, 0);		// reset default scale
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG4, 0x67);	// enable x, y, & z @ 100Hz

	
		
		return true;
	}
	else
	{
		return false;
	}		
}

void app_accel_exercise(void)
{
	app_accel_out_t out;
	int8_t temp;
	
	// Sample Query
	app_accel_query(&out);
	temp = out.t;
	temp = temp * 1.8 + 32;
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "LIS3DSH Temperature: %d F", temp);
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "x = %d, y = %d, z = %d", out.x, out.y, out.z);
			
	// loop a few more measurements
	for (int c=0; c<10; c++)
	{
		app_accel_query(&out);
		bsp_logcat_printf(BSP_LOGCAT_ACCEL, "x = %d, y = %d, z = %d", out.x, out.y, out.z);
		bsp_delay(1);
	}
}

void app_accel_query(app_accel_out_t *out)
{
	uint8_t val;
	uint8_t buf[6];
	
	do
	{
		bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_STATUS, &val);
	} while ((val & 0x80) == 0);
	
	bsp_i2c_read_bytes(BSP_ACCEL_I2C_ADDR, LIS3DSH_OUT_X, buf, sizeof(buf));
	out->x = (int16_t)(buf[1] << 8 | buf[0]);
	out->y = (int16_t)(buf[3] << 8 | buf[2]);
	out->z = (int16_t)(buf[5] << 8 | buf[4]);
	
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_OUT_T, &val);
	out->t = (int8_t)val + 25;		// temperature in C (0 == 25)
	//out->t = out->t * 1.8 + 32;		// temperature in F
	
	out->b = app_accel_read_bandwidth();
	
	out->r = app_accel_read_range();
	
	out->th = app_accel_get_movement_threshold();
}

void app_accel_set_movement_threshold(uint16_t threshold)
{
	if (threshold != 0)
	{
		uint8_t range, reg;
		// set the range for the given threshold
		//app_accel_write_range(threshold / 1000);
		
		// calculate actual register value for new range
		range = app_accel_read_range();
		reg = threshold / range * 128 / 1000;
		if (reg > 0x7f) reg = 0x7f;
		bsp_logcat_printf(BSP_LOGCAT_ACCEL, "Accel State Machine: threshold=%d, range=%d, reg=%02x", threshold, range*1000, reg);
			
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_THRS1_1, reg);		// threshold
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_MASK1_A, 0xfc);		// xyz +/- enabled
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_MASK1_B, 0xfc);		// xyz +/- enabled
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_SETT1, 0x01);		// CONT generates interrupt
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_PR1, 0x00);			// program / reset pointers
			
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_ST1_1, 0x05);		// GNTH1
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_ST1_2, 0x11);		// CONT
			
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG3, 0x48);	// IEA active high, interrupt latched, INT1_EN
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG1, 0x01);	// enable SM1	
	}
	else
	{
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_THRS1_1, 0x00);		// threshold
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG3, 0x00);	// Interrupts disabled
		bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG1, 0x00);	// disable SM1
	}

}

uint16_t app_accel_get_movement_threshold(void)
{
	uint8_t reg, range;
	
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_THRS1_1, &reg);
	range = app_accel_read_range();
	
	//reg = threshold / range * 128 / 1000;
	return reg * 1000 * range / 128;
}

void app_accel_ack(void)
{
	uint8_t out;
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_OUTS1, &out);
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "LIS3DSH SM1 Out: %2x", out);
}

uint8_t app_accel_read_register(uint8_t reg)
{
	uint8_t val;
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, reg, &val);
	return val;
}

void app_accel_write_register(uint8_t reg, uint8_t val)
{
	bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, reg, val);
}

uint8_t app_accel_read_range(void)
{
	uint8_t reg;
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG5, &reg);
	reg = (reg & 0x38) >> 3;	// FSCALE2:0 in bits 3-5
	switch (reg)
	{
		case 0: return 2; break;
		case 1: return 4; break;
		case 2: return 6; break;
		case 3: return 8; break;
		case 4: return 16; break;
	}
	return 0;
}

void app_accel_write_range(uint8_t range)
{
	uint8_t reg, fscale;
	
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "Accel Range: range=%d", range);
	
	// set the range so that it covers the requested range
	// e.g. if you call with range=1 will set 2, range=3 will set 4, etc.
	if (range >= 16)
		fscale = 4;		// +/- 16G
	else if (range >= 8)
		fscale = 3;		// +/- 8G
	else if (range >= 6)
		fscale = 2;		// +/- 6G
	else if (range >= 4)
		fscale = 1;		// +/- 4G
	else
		fscale = 0;		// +/- 2G
		
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG5, &reg);
	//bsp_logcat_printf(BSP_ACCEL_I2C_ADDR, "LIS3DSH_CTRL_REG5=%02x", reg);
	reg = (reg & 0xc3) | (fscale << 3);
	bsp_logcat_printf(BSP_ACCEL_I2C_ADDR, "LIS3DSH_CTRL_REG5=%02x", reg);
	bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG5, reg);
}

uint16_t app_accel_read_bandwidth(void)
{
	uint16_t bw = 0;
	uint8_t val;
	
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG4, &val);
	val = val >> 4;			// ODR3:0 in upper nibble
	switch(val)
	{
		case 0: bw = 0; break;
		case 1: bw = 3; break;
		case 2: bw = 6; break;
		case 3: bw = 12; break;
		case 4: bw = 25; break;
		case 5: bw = 50; break;
		case 6: bw = 100; break;
		case 7: bw = 400; break;
		case 8: bw = 800; break;
		case 9: bw = 1600; break;
	}
	
	return bw;
}

void app_accel_write_bandwidth(uint16_t bandwidth)
{
	uint8_t reg, bw;
	
	if (bandwidth >= 1600)
		bw = 9;
	else if (bandwidth >= 800)
		bw = 8;
	else if (bandwidth >= 400)
		bw = 7;
	else if (bandwidth >= 100)
		bw = 6;
	else if (bandwidth >= 50)
		bw = 5;
	else if (bandwidth >= 25)
		bw = 4;
	else if (bandwidth >= 12)
		bw = 3;
	else if (bandwidth >= 6)
		bw = 2;
	else if (bandwidth >= 3)
		bw = 1;
	else 
		bw = 0;
		
	bsp_i2c_read_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG4, &reg);
	//bsp_logcat_printf(BSP_ACCEL_I2C_ADDR, "LIS3DSH_CTRL_REG4=%02x", reg);
	reg = (reg & 0x0f) | (bw << 4);
	bsp_logcat_printf(BSP_ACCEL_I2C_ADDR, "LIS3DSH_CTRL_REG4=%02x", reg);
	bsp_i2c_write_byte(BSP_ACCEL_I2C_ADDR, LIS3DSH_CTRL_REG4, reg);
}

#endif // BSP_ENABLE_LIS3DSH