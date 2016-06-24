/**
 *	@file	accel_mpu6050.c
 *
 *	@brief	InvenSense MPU-6050 Accelerometer Handler
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

#ifdef BSP_ENABLE_MPU_6050

void accel_mpu6050_crash_detector(void);

#define MPU6050_GYRO_CONFIG     0x1b    // r/w
#define MPU6050_ACCEL_CONFIG    0x1c    // r/w
#define MPU6050_ACCEL_XOUT_H    0x3b	// r
#define MPU6050_PWR_MGMT_1		0x6b	// r/w
#define MPU6050_PWR_MGMT_2		0x6c	// r/w
#define MPU6050_WHO_AM_I		0x75	// r

typedef struct
{
	int16_t x_accel;
	int16_t y_accel;
	int16_t z_accel;
	int16_t temperature;
	int16_t x_gyro;
	int16_t y_gyro;
	int16_t z_gyro;

} mpu_6500_basic_raw_t;
static mpu_6500_basic_raw_t mpu_6500_basic_raw;

static uint32_t accel_get_vector(void)
{
	uint32_t g;
	bsp_i2c_read_bytes(0x68, MPU6050_ACCEL_XOUT_H, (uint8_t *)&mpu_6500_basic_raw, sizeof(mpu_6500_basic_raw));
	g = mpu_6500_basic_raw.x_accel * mpu_6500_basic_raw.x_accel +
	mpu_6500_basic_raw.y_accel * mpu_6500_basic_raw.y_accel +
	mpu_6500_basic_raw.z_accel * mpu_6500_basic_raw.z_accel;
	return g;
}

void accel_mpu6050_crash_detector(void)
{
	uint8_t val;
	uint32_t baseg, currentg;
	bool crashed = false;
	
	bsp_i2c_read_byte(0x68, MPU6050_WHO_AM_I, &val);
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "MPU-6050 Query: %02X", val);
	val = 0;		// clear sleep mode
	bsp_i2c_write_byte(0x68, MPU6050_PWR_MGMT_1, val);
	bsp_i2c_read_byte(0x68, MPU6050_PWR_MGMT_1, &val);
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "MPU-6050 Power Mgmt: %02X", val);
	val = 0x08;	// +/- 4G
	bsp_i2c_write_byte(0x68, MPU6050_ACCEL_CONFIG, val);
		

	bsp_logcat_print(BSP_LOGCAT_ACCEL, "MPU-6050 Getting Baseline");
	baseg = accel_get_vector();
	for (int i=0; i<32; i++)
	{
		currentg = accel_get_vector();
			
		//baseg = (currentg + (baseg * 63)) >> 6;	// 1/64th new
		baseg = (currentg + (baseg * 7)) >> 3;		// 1/8 new
		bsp_delay(1);
	}
	bsp_logcat_printf(BSP_LOGCAT_ACCEL, "MPU-6050 Baseline Vector: %d", baseg);
	bsp_delay(50);		// give up the CPU for logcat

	while (1)
	{
		currentg = accel_get_vector();
		if (!crashed && abs(currentg - baseg) > 100000000)
		{
			app_pdg_accelerometer_event();
			bsp_logcat_printf(BSP_LOGCAT_ACCEL, "CRASH!!! %d %d %d", currentg, baseg, abs(currentg - baseg));

			crashed = true;
		}
		else if (abs(currentg - baseg) < 100000000)
		{
			crashed = false;
		}
		baseg = (currentg + (baseg * 7)) >> 3;		// 1/8 new
		//bsp_logcat_printf(BSP_LOGCAT_INFO, "MPU-6080 %d %d", currentg, baseg);
		bsp_delay(1);
	}
}


/*
	The following are stub just to be able to build P1
*/
uint8_t app_accel_read_register(uint8_t reg)
{
	(void)reg;
	return 0xff;
}

void app_accel_write_register(uint8_t reg, uint8_t val)
{
	(void)reg;
	(void)val;
}

void app_accel_ack(void)
{
}

void app_accel_set_movement_threshold(uint16_t threshold)
{
	(void)threshold;
}	

void app_accel_query(app_accel_out_t *out)
{
	out->t = 25;
	out->x = 0;
	out->y = 0;
	out->z = -16384;
	out->b = 100;
	out->r = 2;
	out->th = 1500;
}

bool app_accel_init_dev(void)
{	
	return true;
}

void app_accel_write_bandwidth(uint16_t bandwidth)
{
	(void)bandwidth;
}

void app_accel_write_range(uint8_t range)
{
	(void)range;
}
#endif // BSP_ENABLE_MPU_6050