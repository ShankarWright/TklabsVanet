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

#include <string.h>
#include <stdlib.h>
#include <asf.h>
#include "vanet.h"

#define ADC_DEFAULT_CAL_OFFSET			0x7fff

const bsp_codeplug_t bsp_defaults = {
	/* factory */
	{
		/* cp_signature_1 */			BSP_CP_SIGNATURE_1,
		
		/* adc0_offset_cal */           ADC_DEFAULT_CAL_OFFSET,
		/* adc0_gain_cal_num */         0,
		/* adc0_gain_cal_dem */         1000,
		/* adc1_offset_cal */           ADC_DEFAULT_CAL_OFFSET,
		/* adc1_gain_cal_num */         0,
		/* adc1_gain_cal_dem */         1000,
		/* dac0_offset_cal */           0x1e0,
		/* dac0_gain_cal */             0x7f,
		/* dac1_offset_cal */           0x1e0,
		/* dac1_gain_cal */             0x7f,
			
		/* factory_pad[6] */			{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		/* factory_version */			1
	},	
	
    /* cp_signature */                  BSP_CP_SIGNATURE,
	
	/* logcat_store_mask */				0xffff,
	/* logcat_print_mask */				0x0007,
	
	/* accel_thresh */					1900,
	
	// NOTE: fan_speed[0]/ambient_temp[0] control the minimum fan pwm, fan_speed[3] controls the maximum
	/* ambient_temp */					{ 25,		35,		40,		50 },
	/* fan_speed */						{ 20,		50,		75,		100 },	
	/* fan_control_period */			5000,
    
	/* pad[10] */						{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	/* defaults_version */				3
};

#define STRUCT_OFFSET(structure, object)    ((unsigned int) &(((structure *)NULL)->object))
#define STRUCT_SIZE(structure, object)      (sizeof(((structure *)NULL)->object))

#define DEFINE_CP_FIELD(_field, _type, _desc) { #_field, \
												STRUCT_OFFSET(bsp_codeplug_t, _field), \
												STRUCT_SIZE(bsp_codeplug_t, _field), \
												_type, \
												false, \
                                                _desc }
												
cp_field_t cp_fields[] =
{
	// logcat
	DEFINE_CP_FIELD(logcat_store_mask,			CP_FIELD_HEX,	"Logcat to Memory Mask"),
	DEFINE_CP_FIELD(logcat_print_mask,			CP_FIELD_HEX,	"Logcat Default Display Mask"),
	
	// accelerometer threshold
	DEFINE_CP_FIELD(accel_thresh,				CP_FIELD_UINT,	"Accelerometer Threshold (1000=1G)"),
	
	// fan control
	DEFINE_CP_FIELD(fan_control_period,			CP_FIELD_UINT,	"Fan Control Period (ms)"),
	DEFINE_CP_FIELD(ambient_temp,				CP_FIELD_HEX,	"Fan Control Ambient Temperature Table"),
	DEFINE_CP_FIELD(fan_speed,					CP_FIELD_HEX,	"Fan Control Fan Speed Table"),
	
	// adc
	DEFINE_CP_FIELD(factory.adc0_offset_cal,	CP_FIELD_INT,   "ADC0 offset calibration"),
	DEFINE_CP_FIELD(factory.adc0_gain_cal_num,  CP_FIELD_INT,   "ADC0 gain calibration numerator"),
	DEFINE_CP_FIELD(factory.adc0_gain_cal_den,  CP_FIELD_UINT,  "ADC0 gain calibration denominator"),
	DEFINE_CP_FIELD(factory.adc1_offset_cal,	CP_FIELD_INT,   "ADC1 offset calibration"),
	DEFINE_CP_FIELD(factory.adc1_gain_cal_num,  CP_FIELD_INT,   "ADC1 gain calibration numerator"),
	DEFINE_CP_FIELD(factory.adc1_gain_cal_den,  CP_FIELD_UINT,  "ADC1 gain calibration denominator"),
};

#define BSP_CP_NUM_RW_FIELDS (sizeof(cp_fields)/sizeof(cp_field_t))

uint8_t cp_print_num_fields(void)
{
	return BSP_CP_NUM_RW_FIELDS;
}

void cp_print_field(uint8_t field)
{
	uint32_t uint_data;
	int32_t int_data;
	//uint8_t *p_cp = cp_fields[field].app ? (uint8_t *)p_user_codeplug : (uint8_t *)p_bsp_codeplug;
	uint8_t *p_cp = (uint8_t *)p_bsp_codeplug;
	
	print_dbgn(cp_fields[field].name, 28);
	if (cp_fields[field].format == CP_FIELD_UINT)
	{
		print_dbg("(uns) ");
		if (cp_fields[field].size == 1)
		uint_data = *(uint8_t *)(p_cp + cp_fields[field].offset);
		else if (cp_fields[field].size == 2)
		uint_data = *(uint16_t *)(p_cp + cp_fields[field].offset);
		else if (cp_fields[field].size == 4)
		uint_data = *(uint32_t *)(p_cp + cp_fields[field].offset);
		else
		uint_data = 1000800001;
		
		print_dbg_int(uint_data);
	}
	else if (cp_fields[field].format == CP_FIELD_INT)
	{
		print_dbg("(int) ");
		if (cp_fields[field].size == 1)
		int_data = *(int8_t *)(p_cp + cp_fields[field].offset);
		else if (cp_fields[field].size == 2)
		int_data = *(int16_t *)(p_cp + cp_fields[field].offset);
		else if (cp_fields[field].size == 4)
		int_data = *(int32_t *)(p_cp + cp_fields[field].offset);
		else
		int_data = 1000800001;
		
		print_dbg_int(int_data);
	}
	else if (cp_fields[field].format == CP_FIELD_HEX)
	{
		print_dbg("(hex) ");
		print_dbg_array_plain(p_cp + cp_fields[field].offset, cp_fields[field].size);
	}
	
	print_dbg("    -> ");
	print_dbg(cp_fields[field].desc);
	print_dbg("\r\n");
}