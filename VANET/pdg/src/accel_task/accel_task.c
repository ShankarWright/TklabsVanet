/**
 *	@file	accel_task.c
 *
 *	@brief	Accelerometer Handler Task
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

static OS_STK s_app_accel_task_stack[TASK_ACCEL_STACK_SIZE];
static void accel_task(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[16];

enum
{
	APP_ACCEL_EVENT_TIMER		= 0x0001,
};
static bsp_tkvs_timer_t s_timer;

#ifdef BSP_ENABLE_MPU_6050
extern void accel_mpu6050_crash_detector(void);
#endif

void app_accel_task_init()
{
	INT8U perr;
	
	// debug console message
	print_dbg("Creating Accelerometer Task\r\n");
		
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "Accelerometer task: OSQCreate");
		
	// Create the task
	OSTaskCreateExt(accel_task,
		(void *)0,
		(OS_STK *)&s_app_accel_task_stack[TASK_ACCEL_STACK_SIZE - 1],
		TASK_ACCEL_PRIO,
		TASK_ACCEL_PRIO,
		(OS_STK *)&s_app_accel_task_stack[0],
		TASK_ACCEL_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OS_CHECK_PERR(perr, "Accelerometer task: OSTaskCreateExt");
		
	OSTaskNameSet(TASK_ACCEL_PRIO, (INT8U *)"Accel Task", &perr);
	OS_CHECK_PERR(perr, "Accelerometer task: OSTaskNameSet");
	
	// subscribe to our sources
	bsp_tkvs_subscribe(APP_TKVS_ACCEL_TASK, BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_ACCEL_PRIO);
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_ACCELEROMETER_RAW), 
			BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_ACCEL_PRIO);
}

static void accel_task(void *p_arg)
{
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	
	(void) p_arg;
		
#ifdef BSP_ENABLE_MPU_6050
	accel_mpu6050_crash_detector();		// this will block forever... test code for P1
#endif

#ifdef BSP_ENABLE_LIS3DSH
	if (app_accel_init_dev())
	{
		// exercise some readings
		app_accel_exercise();
		
		// enable basic movement detection
		app_accel_set_movement_threshold(bsp_cp_get_field(accel_thresh));
		
		// subscribe to accelerometer events
		bsp_tkvs_subscribe(BSP_TKVS_SRC_PIN, BSP_PIN_EVENT_ACCEL, s_msg_flag, TASK_ACCEL_PRIO);
	}
#endif
	
	bsp_tkvs_init_timer(&s_timer, BSP_TKVS_TIMER_PERIODIC, 0, APP_TKVS_ACCEL_TASK, 
		APP_ACCEL_EVENT_TIMER, BSP_TKVS_TIMER_MS_TO_TICKS(1000));
	
	// Task Loop
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);    // block forever on my queue
		if (msg != (void *)0)
		{
			if (msg->source == BSP_TKVS_SRC_PIN && msg->event == BSP_PIN_EVENT_ACCEL)
			{
				bsp_logcat_printf(BSP_LOGCAT_ACCEL, "Accel Event %d", msg->immed_data);
				if (msg->immed_data == 1)
				{
					app_pdg_accelerometer_event();
					// re-arm
					app_accel_ack();
				}
			}	
			else if (msg->source == BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_ACCELEROMETER_RAW))		
			{
				// somebody wants some raw data
				if (msg->event == BSP_MUX_EVENT_CONNECT)
				{
					bsp_logcat_print(BSP_LOGCAT_INFO, "Starting Raw Accelerometer Data");
					bsp_tkvs_start_timer(&s_timer);
				}	
				else if (msg->event == BSP_MUX_EVENT_DISCONNECT)
				{
					bsp_logcat_print(BSP_LOGCAT_INFO, "Stopping Raw Accelerometer Data");
					bsp_tkvs_stop_timer(&s_timer);
				}
			}
			else if (msg->source == APP_TKVS_ACCEL_TASK && msg->event == APP_ACCEL_EVENT_TIMER)
			{
				char msg[40];
				app_accel_out_t out;
				app_accel_query(&out);
				dlib_snprintf(msg, sizeof(msg), "t=%d x=%d y=%d z=%d\r\n", out.t, out.x, out.y, out.z);
				bsp_mux_send(VANET_MUXCH_ACCELEROMETER_RAW, msg, strlen(msg));
			}											
				
			bsp_tkvs_free(msg);
		}
	}
}