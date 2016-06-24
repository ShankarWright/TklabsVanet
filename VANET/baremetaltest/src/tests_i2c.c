/**
 *	@file	tests.c
 *
 *	@brief	Test routines code
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

#include <string.h>
#include <asf.h>
#include "tests.h"
#include "vanet.h"



/************************************************************************/
/* TWIM Tests                                                           */
/************************************************************************/
static bool s_twim_inititialized = false;
void twim_init(void)
{
	if (!s_twim_inititialized)
	{
		print_dbg("Initializing TWIMS0 Master...\r\n");
		bsp_i2c_init();
		//gpio_configure_pin(BSP_INIT_TWIM_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
		s_twim_inititialized = true;
	}		
}

void twim_discover_devices(void)
{
	twim_init();
			
	print_dbg("Slaves found: ");
	for (int i=1; i<127; i++)
	{
		status_code_t status = twim_probe(&AVR32_TWIM0, i);
		if (status == STATUS_OK)
		{
			print_dbg_hex(i);
			print_dbg(" ");
		}
		else
		{
			print_dbg(" - ");
		}
	}
	print_dbg("\r\n");
}

/************************************************************************/
/* MPU-6500                                                             */
/************************************************************************/
#define MPU6050_GYRO_CONFIG     0x1b    // r/w
#define MPU6050_ACCEL_CONFIG    0x1c    // r/w
#define MPU6050_ACCEL_XOUT_H    0x3b	// r
#define MPU6050_PWR_MGMT_1		0x6b	// r/w
#define MPU6050_PWR_MGMT_2		0x6c	// r/w
#define MPU6050_WHO_AM_I		0x75	// r
#define SWAP(_uint16) ((uint16_t)( ((0xff00 & _uint16) >> 8) | ((0xff & _uint16) << 8)))
uint8_t mpubuf[16];
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
mpu_6500_basic_raw_t mpu_6500_basic_raw;
void mpu_6500_basic(void)
{
	twim_init();
	
	status_code_t status;
	twim_package_t mpupkt = {
		.chip = 0x68,
		.addr[0] = 0,		// fill in later
		.addr_length = 1,
		.buffer = mpubuf,
		.length = 0,		// fill in later
	};
		
	// read MPU-6050 ID
	mpupkt.addr[0] = MPU6050_WHO_AM_I;
	mpupkt.length = 1;
	status = twim_read_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Read MPU6050_WHO_AM_I (status=");
	print_dbg_int(status); print_dbg(") :"); print_dbg_hex(mpubuf[0]); print_dbg("\r\n");
	
	if (mpubuf[0] != 0x68)
	{
		print_dbg("MPU-6050 Not Found...\r\n");
		return;		
	}
	
	// read current gyro / accel config
	mpupkt.addr[0] = MPU6050_GYRO_CONFIG;
	mpupkt.length = 2;
	status = twim_read_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Read MPU6050_GYRO/ACCEL_CONFIG (status=");
	print_dbg_int(status); print_dbg(") :"); 
	print_dbg_hex(mpubuf[0]); print_dbg(" "); print_dbg_hex(mpubuf[1]);
	print_dbg("\r\n");
	
	// read current sleep state
	mpupkt.addr[0] = MPU6050_PWR_MGMT_1;
	mpupkt.length = 1;
	status = twim_read_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Read MPU6050_PWR_MGMT_1 (status=");
	print_dbg_int(status); print_dbg(") :"); print_dbg_hex(mpubuf[0]); print_dbg("\r\n");
	
	// clear the sleep bit
	mpupkt.addr[0] = MPU6050_PWR_MGMT_1;
	mpupkt.length = 1;
	mpubuf[0] = 0;		// clear SLEEP
	status = twim_write_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Write MPU6050_PWR_MGMT_1 (status=");
	print_dbg_int(status); print_dbg(")\r\n");
	
	// read new sleep state
	mpupkt.addr[0] = MPU6050_PWR_MGMT_1;
	mpupkt.length = 1;
	status = twim_read_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Read MPU6050_PWR_MGMT_1 (status=");
	print_dbg_int(status); print_dbg(") :"); print_dbg_hex(mpubuf[0]); print_dbg("\r\n");
	
	// And read some raw measurements!
	mpupkt.addr[0] = MPU6050_ACCEL_XOUT_H;
	mpupkt.buffer = &mpu_6500_basic_raw;
	mpupkt.length = sizeof(mpu_6500_basic_raw);
	status = twim_read_packet(&AVR32_TWIM0, &mpupkt);
	print_dbg("MPU Read MPU6050_ACCEL_XOUT_H (status=");
	print_dbg_int(status); print_dbg(")\r\n");
	print_dbg("MPU Temp=");
	print_dbg_int(mpu_6500_basic_raw.temperature / 340 + 36);
	print_dbg("\r\n");
	
	// Loop raw measurements until a key is hit
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		twim_read_packet(&AVR32_TWIM0, &mpupkt);
		//mpu_6500_basic_raw.x_gyro = SWAP(mpu_6500_basic_raw.x_gyro);
		//mpu_6500_basic_raw.y_gyro = SWAP(mpu_6500_basic_raw.y_gyro);
		//mpu_6500_basic_raw.z_gyro = SWAP(mpu_6500_basic_raw.z_gyro);
		print_dbg_int(mpu_6500_basic_raw.temperature / 340 + 36);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.x_accel);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.y_accel);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.z_accel);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.x_gyro);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.y_gyro);
		print_dbg(" ");
		print_dbg_int(mpu_6500_basic_raw.z_gyro);
		print_dbg("\r\n");
		cpu_delay_ms(500, sysclk_get_cpu_hz());
	}
}

#define LIS3DSH_WHO_AM_I		0x0f	// r
#define LIS3DSH_OUT_X_L			0x28
#define LIS3DSH_OUT_T			0x0c
void lis3dsh_basic(void)
{
	uint8_t val;
	int8_t temp_c, temp_f;
	uint8_t out[6];
	
	twim_init();
	bsp_i2c_read_byte(0x1d, LIS3DSH_WHO_AM_I, &val);
	if (val != 0x3f)
	{
		print_dbg("LIS3DSH Not Found...\r\n");
		return;
	}
	bsp_i2c_read_byte(0x1d, LIS3DSH_OUT_T, &val);
	temp_c = (int8_t)val;
	temp_c += 25;
	temp_f = temp_c * 1.8 + 32;
	print_dbg("Temp="); print_dbg_int(temp_f); print_dbg("\r\n");
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		bsp_i2c_read_bytes(0x1d, LIS3DSH_OUT_X_L, out, 6);
		print_dbg_int((int16_t)(out[1] << 8 | out[0]));
		print_dbg(" ");
		print_dbg_int((int16_t)(out[3] << 8 | out[2]));
		print_dbg(" ");
		print_dbg_int((int16_t)(out[5] << 8 | out[4]));
		print_dbg("\r\n");
		cpu_delay_ms(500, sysclk_get_cpu_hz());
	}		
}


/************************************************************************/
/* PCA9501 Tests                                                        */
/************************************************************************/
void i2c_pca9501_test_eeprom(void)
{
	twim_init();
	
	status_code_t status;
	uint8_t i2c_data[16];
	/*
	for (int i=0; i<10; i++)
	{
		print_dbg("PCA9501 Output High\r\n");
		i2c_data[0] = 0xff;
		status = twim_write(&AVR32_TWIM0, i2c_data, 1, 0x3f, false);
		cpu_delay_ms(5000, sysclk_get_cpu_hz());
		print_dbg("PCA9501 Output Low\r\n");
		i2c_data[0] = 0x00;
		status = twim_write(&AVR32_TWIM0, i2c_data, 1, 0x3f, false);
		cpu_delay_ms(5000, sysclk_get_cpu_hz());
	}		
	*/
	/*
	i2c_data[0] = 0;	// addr
	twim_write(&AVR32_TWIM0, i2c_data, 1, 0x1f | 0x40, false);	// 0x40 for eeprom
	for (int i=0; i<16; i++)
		i2c_data[i] = 0xaa;
			
	twim_read(&AVR32_TWIM0, i2c_data, 16, 0x1f | 0x40, false);
	for (int i=0; i<16; i++)
	{
		print_dbg_hex(i2c_data[i]);
		print_dbg("\r\n");
	}		
	*/
	twim_package_t eerdwr = {
		.chip = 0x1f | 0x40,
		.addr[0] = 0,
		.addr_length = 1,
		.buffer = i2c_data,
		.length = 16,
	};
	for (int i=0; i<16; i++)
		i2c_data[i] = 15-i;
	eerdwr.addr[0] = 0;	// write starting at 0
	status = twim_write_packet(&AVR32_TWIM0, &eerdwr);
	print_dbg("PCA9501 EE Write Status = ");
	print_dbg_int(status);
	print_dbg("\r\n");
			
	for (int i=0; i<16; i++)
		i2c_data[i] = 0xaa;
		
	eerdwr.addr[0] = 4;	// read starting at 4
	status = twim_read_packet(&AVR32_TWIM0, &eerdwr);
	print_dbg("PCA9501 EE Read Status = ");
	print_dbg_int(status);
	print_dbg("\r\n");
	for (int i=0; i<16; i++)
	{
		print_dbg_hex(i2c_data[i]);
		print_dbg("\r\n");
	}
}

void ds1077_test(void)
{
	twim_init();
		
	print_dbg("Setting MUX to POR Value 0x1800\r\n");
	bsp_i2c_write_2bytes_verbose(0x58, 0x02, 0x18, 0x00);
	
	print_dbg("Setting DIV to POR Value 0x0000\r\n");
	bsp_i2c_write_2bytes_verbose(0x58, 0x01, 0x00, 0x00);
	
	print_dbg("Setting BUS to disable writes to EEPROM\r\n");
	bsp_i2c_write_byte_verbose(0x58, 0x0d, 0x08);
	
	print_dbg("Setting OUT1 Prescaler to /4\r\n");
	bsp_i2c_write_2bytes_verbose(0x58, 0x02, 0x19, 0x00);
	
	// default is 133.333MHz / (DIV+2) == 133.333MHz / 2 = 66.667MHz
	uint32_t div = 302;
	uint8_t bytes[2];
	div = div - 2;
	div = div << 6;
	bytes[0] = (div >> 8) & 0xff;		// high byte
	bytes[1] = (div & 0xff);			// low byte
	
	print_dbg("Setting DIV to 302 for a OUT1 of 110375Hz\r\n");
	bsp_i2c_write_2bytes_verbose(0x58, 0x01, bytes[0], bytes[1]);
}

void max17043_test(void)
{
	uint8_t ver[2];
	uint8_t vcell[2]; 
	uint8_t soc[2];
	uint16_t cellv;
	
	twim_init();
	
	bsp_i2c_read_bytes(0x36, 0x08, ver, 2);
	print_dbg("MAX17043 Version :"); print_dbg_array_plain(ver, 2); print_dbg("\r\n");
	
	bsp_i2c_read_bytes(0x36, 0x02, vcell, 2);
	cellv = (vcell[0] << 4) + (vcell[1] >> 4);
	print_dbg("VCell :"); print_dbg_array_plain(vcell, 2); print_dbg(" "); print_dbg_int(cellv * 1.25); print_dbg("mv\r\n");
	
	bsp_i2c_read_bytes(0x36, 0x04, soc, 2);
	print_dbg("SOC :"); print_dbg_array_plain(soc, 2); print_dbg(" "); print_dbg_int(soc[0]); print_dbg("%\r\n");
	
	int ch;
	while (usart_read_char(DBG_USART, &ch) == USART_RX_EMPTY)
	{
		bsp_i2c_read_bytes(0x36, 0x02, vcell, 2);
		bsp_i2c_read_bytes(0x36, 0x04, soc, 2);
		cellv = (vcell[0] << 4) + (vcell[1] >> 4);
		print_dbg("VCell :"); print_dbg_int(cellv * 1.25); print_dbg("mv "); print_dbg_int(soc[0]); print_dbg("%\r\n");
		cpu_delay_ms(1000, sysclk_get_cpu_hz());
	}
}
