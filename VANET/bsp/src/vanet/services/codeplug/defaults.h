/**
 *	@file	defaults.c
 *
 *	@brief	Codeplug Defaults
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

enum {
	CP_FIELD_INT,
	CP_FIELD_UINT,
	CP_FIELD_HEX
};

void cp_print_field(uint8_t field);

uint8_t cp_print_num_fields(void);

typedef const struct {
	const char  *name;
	uint8_t     offset;
	uint8_t     size;
	uint8_t     format;
	uint8_t     app;
	const char *desc;
} cp_field_t;

extern cp_field_t cp_fields[];

/// Factory codeplug data
typedef const struct
{
	// signature
	uint32_t	cp_signature_1;							// 0-3
	
	// avr32 calibration
	int16_t		adc0_offset_cal;						// 4-5
	int16_t		adc0_gain_cal_num;						// 6-7
	uint16_t    adc0_gain_cal_den;						// 8-9
	int16_t		adc1_offset_cal;						// 10-11
	int16_t		adc1_gain_cal_num;						// 12-13
	uint16_t    adc1_gain_cal_den;						// 14-15
	uint16_t	dac0_offset_cal;						// 16-17
	uint16_t	dac0_gain_cal;							// 18-19
	uint16_t	dac1_offset_cal;						// 20-21
	uint16_t	dac1_gain_cal;							// 22-23
	
	// unused pad
	uint8_t		factory_pad[6];							// 24-29
	
	// factory version
	uint16_t	factory_version;						// 30-31
} bsp_factory_codeplug_t;

/// Structure of BSP codeplug data - Take care to align your data types and always pad to 16 bytes
typedef const struct
{
	// factory settings
	bsp_factory_codeplug_t factory;						// 0-31
	// signature
	uint32_t    cp_signature;                           // 32-35
	// logcat
	uint16_t	logcat_store_mask;						// 36-37
	uint16_t	logcat_print_mask;						// 38-39

	// accelerometer threshold
	uint16_t	accel_thresh;							// 40-41
	
	// fan controller
	uint8_t		ambient_temp[4];						// 42-45
	uint8_t		fan_speed[4];							// 46-49
	uint16_t	fan_control_period;						// 50-51
	
	// unused pad
	uint8_t		pad[10];								// 52-61
	// defaults version
	uint16_t	defaults_version;						// 62-63

} bsp_codeplug_t;