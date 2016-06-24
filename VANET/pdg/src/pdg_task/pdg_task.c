/**
 *	@file	pdg_task.c
 *
 *	@brief	Peripheral Data Gateway Task
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
#include "pdg_cmd.h"
#include "conf_tones.h"

static OS_STK s_pdg_task_stack[TASK_PDG_STACK_SIZE];
static void pdg_task(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[16];

enum
{
	APP_PDG_EVENT_TIMER		= 0x0001,
};

static bsp_clcd_t *lcd = NULL;
static const bsp_clcd_interface_i2c_pcf8574_t lcd_i2c_params =
{
	.i2c_addr = 0x3e,
	.en = 2,
	.rw = 1,
	.rs = 0,
	.bl = 3,				// gpio 3
};

static bsp_tkvs_timer_t s_timer;

void app_pdg_task_init()
{
	INT8U perr;
	
	// debug console message
	print_dbg("Creating PDG Task\r\n");
	
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "PDG task: OSQCreate");
	
	// Create the task
	OSTaskCreateExt(pdg_task,
		(void *)0,
		(OS_STK *)&s_pdg_task_stack[TASK_PDG_STACK_SIZE - 1],
		TASK_PDG_PRIO,
		TASK_PDG_PRIO,
		(OS_STK *)&s_pdg_task_stack[0],
		TASK_PDG_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OS_CHECK_PERR(perr, "PDG task: OSTaskCreateExt");
	
	OSTaskNameSet(TASK_PDG_PRIO, (INT8U *)"PDG Task", &perr);
	OS_CHECK_PERR(perr, "PDG task: OSTaskNameSet");
	
	// subscribe to our sources
	bsp_tkvs_subscribe(APP_TKVS_PDG_TASK, BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_PDG_PRIO);
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_UNIFIED), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_PDG_PRIO);
	bsp_tkvs_subscribe(BSP_TKVS_SRC_PIN, BSP_PIN_EVENT_SW_PB02, s_msg_flag, TASK_PDG_PRIO);
}

static uint8_t s_spinner_cnt = 0;
static const char s_spinner[] = { '.', 'o', 'O', '@', '*', ' ' };
	
static void pdg_task(void *p_arg)
{
	app_accel_out_t out;
	uint8_t last_temp = 100;
	uint16_t new_temp;
	uint8_t last_fan_speed = 0;
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	uint8_t ambient_temp[4];
	uint8_t fan_speed[4];
	
    // powerup sound
    #ifdef CONFIG_BSP_ENABLE_ALERT
    bsp_alert(BSP_ALERT_STARTUP);
    #endif

	// Create our LCD
	lcd = bsp_clcd(BSP_CLCD_HD44780, BSP_CLCD_I2C_PCF8574, (void *)&lcd_i2c_params);
	if (lcd)
	{
		bsp_clcd_setup(lcd, 20, 4);
		bsp_clcd_backlight(lcd, true);
		bsp_clcd_display(lcd, true);
		bsp_clcd_clear(lcd);
		bsp_clcd_write_line(lcd, 0, BSP_CLCD_ALIGN_CENTER, "VANET FAU/tkLABS");
	}
	else
	{
		bsp_logcat_print(BSP_LOGCAT_CRITICAL, "Could not create LCD");
	}

	// Start a timer...
	bsp_tkvs_init_timer(&s_timer, BSP_TKVS_TIMER_PERIODIC, 0, APP_TKVS_PDG_TASK, APP_PDG_EVENT_TIMER, 
		BSP_TKVS_TIMER_MS_TO_TICKS(bsp_cp_get_field(fan_control_period)));
	bsp_tkvs_start_timer(&s_timer);
		
	// Read fan codeplug settings
	memcpy(ambient_temp, bsp_cp_get_field(ambient_temp), sizeof(ambient_temp));
	memcpy(fan_speed, bsp_cp_get_field(fan_speed), sizeof(fan_speed));
	
	// Initialize the PWM for the fan
	bsp_pwm_enable_channel(2, 25000, 0);
	
	// Task Loop
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);    // block forever on my queue
		if (msg != (void *)0)
		{
			if (msg->source == BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_UNIFIED))
			{
				if (msg->event == BSP_MUX_EVENT_DATA_RCVD && BSP_TKVS_MSG_HAS_DATA(msg))
				{
                    uint16_t i;
                    for (i=0; i<msg->data_len; i++) app_pdg_cmd_put(msg->data[i]);
				}
			}
			else if (msg->source == APP_TKVS_PDG_TASK)
			{
				if (msg->event == APP_PDG_EVENT_TIMER)
				{
					if (lcd)
					{
						char text[2] = {'\0', '\0'};
						text[0] = s_spinner[s_spinner_cnt++];
						if (s_spinner_cnt == sizeof(s_spinner)) s_spinner_cnt = 0;
						bsp_clcd_write_pos(lcd, 3, 9, text);
					}
					
					// monitor temperature
					app_accel_query(&out);
					new_temp = ((out.t * 10) + (last_temp * 30)) / 40;	// 1/4 new out.t, 3/4 last_temp
					bsp_logcat_printf(BSP_LOGCAT_NOISE, "%d %d > %d", last_temp, out.t, new_temp);
					if (last_temp != new_temp)
					{
						char temp[21];
						uint8_t new_fan_speed = 0;
						
						last_temp = new_temp;
						
						if (last_temp < ambient_temp[0])
						{
							new_fan_speed = 0;						// minimum fan speed is fan_speed[0]
						}
						else if (last_temp > ambient_temp[3])
						{
							new_fan_speed = fan_speed[3];			// maximum fan speed
						}
						else
						{
							// interpolate this!
							for (int i=1; i<sizeof(ambient_temp); i++)
							{
								uint8_t y0 = fan_speed[i-1];
								uint8_t y1 = fan_speed[i];
								uint8_t x0 = ambient_temp[i-1];
								uint8_t x1 = ambient_temp[i];
								bsp_logcat_printf(BSP_LOGCAT_NOISE, "%d %d %d %d %d", last_temp, x0, x1, y0, y1);
								if (last_temp < x1)
								{
									new_fan_speed = y0 + (y1 - y0) * (last_temp - x0) / (x1 - x0);
									break;
								}
							}	
						}												
							
						if (last_fan_speed != new_fan_speed)
						{
							last_fan_speed = new_fan_speed;
							bsp_pwm_update(2, last_fan_speed);
						}
						
						// update for new temp
						if (last_fan_speed == 0)
							dlib_snprintf(temp, sizeof(temp), "Temp %dC, Fan OFF", last_temp);
						else
							dlib_snprintf(temp, sizeof(temp), "Temp %dC, Fan %d%%", last_temp, last_fan_speed);
						bsp_logcat_print(BSP_LOGCAT_DEBUG, temp);
						bsp_clcd_write_line(lcd, 2, BSP_CLCD_ALIGN_CENTER, temp);
					}
				}					
			}
			else if (msg->source == BSP_TKVS_SRC_PIN)
			{
				// this is temporary-ish
				if (msg->event == BSP_PIN_EVENT_SW_PB02)
				{
					app_pdg_button_event(0, msg->immed_data);
				}					
			}
			bsp_tkvs_free(msg);
		}
	}
}			

void app_pdg_accelerometer_event(void)
{			
#ifdef CONFIG_BSP_ENABLE_ALERT
	bsp_alert(BSP_ALERT_CRASH);
#endif
	app_pdg_send_msg(VANET_GRP_BUTTON_EVENT, VANET_OP_MOTION_INTERRUPT, 0, 0);
}


void app_pdg_button_event(uint8_t button, uint8_t state)
{	
	app_pdg_send_msg(VANET_GRP_BUTTON_EVENT, state ? VANET_OP_BUTTON_PRESS : VANET_OP_BUTTON_RELEASE, &button, 1);
}