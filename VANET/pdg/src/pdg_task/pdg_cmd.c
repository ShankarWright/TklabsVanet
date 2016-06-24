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

static uint8_t s_cmdbuf[256];
static uint16_t s_cmd_bytes_rx;
static uint16_t s_cmd_payload_len;

#define CMDKEY(group,opcode)            ((((group)&0xff)<<8) | ((opcode)&0xff))

inline static uint16_t read16(const uint8_t* buf, int offset)
{
    return ((buf[offset] & 0xff) << 8) | (buf[offset+1] & 0xff);
}

static void cmd_apiversion(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
    uint8_t b = VANET_API_VERSION;
    app_pdg_send_msg(grp, opcode, &b, 1);
}

static void cmd_buzzer(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
    static bsp_buzzer_state_t s_states[16];
    
    if (opcode == VANET_OP_SET_BUZZER_START)
    {
        /*
            Byte 0      - Number of states (max 16)
            Byte 1      - Number of times to repeat the cycle of states (0 for infinite)
            Byte 2-3    - State 1 tone frequency (0 for off)
            Byte 4-5    - State 1 duration in ms (0 for infinite)
            ...
        */

        uint8_t i, n = payload[0];
        if (n > 16) n = 16;
        for (i=0; i<n; i++)
        {
            s_states[i].freq = read16(payload, 4*i + 2);
            s_states[i].ms = read16(payload, 4*i + 4);
        }
        bsp_buzzer_start(BUZZER_SURVEY, s_states, n, payload[1] > 0 ? payload[1] : -1);
    }
    else
    {
        bsp_buzzer_stop();
    }
    
    app_pdg_send_msg(grp, opcode, 0, 0);
}

static void cmd_accelerometer(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
	if (opcode == VANET_OP_ACCELEROMETER_QUERY)
	{
		app_accel_out_t out;
		vanet_api_accel_t accel;
		app_accel_query(&out);
		accel.x_accel = out.x;
		accel.y_accel = out.y;
		accel.z_accel = out.z;
		accel.temperature = out.t;
		accel.bandwidth = out.b;
		accel.range = out.r;
		accel.threshold = out.th;
		
		app_pdg_send_msg(grp, opcode, (const uint8_t *)&accel, 12);
	}
	else if (opcode == VANET_OP_ACCELEROMETER_SET_MOVEMENT_THRESHOLD)
	{
		/*
			Byte 0,1 - Threshold for built in state machine
		*/
		uint16_t threshold = read16(payload, 0);
		app_accel_set_movement_threshold(threshold);
		app_pdg_send_msg(grp, opcode, 0, 0);
	}	
	else if (opcode == VANET_OP_ACCELEROMETER_REGISTER_READ)
	{
		/*
			Byte 0 - Register to Read
		*/
		uint8_t val = app_accel_read_register(payload[0]);
		app_pdg_send_msg(grp, opcode, &val, 1);	
	}
	else if (opcode == VANET_OP_ACCELEROMETER_REGISTER_WRITE)
	{
		/*
			Byte 0 - Register to Write
			Byte 1 - Value
		*/
		app_accel_write_register(payload[0], payload[1]);
		app_pdg_send_msg(grp, opcode, 0, 0);
	}
	else if (opcode == VANET_OP_ACCELEROMETER_SET_BANDWIDTH)
	{
		/* Byte 0,1 Bandwidth */
		uint16_t bandwidth = read16(payload, 0);
		app_accel_write_bandwidth(bandwidth);
		app_pdg_send_msg(grp, opcode, 0, 0);
	}
	else if (opcode == VANET_OP_ACCELEROMETER_SET_RANGE)
	{
		/* Byte 0 Range */
		app_accel_write_range(payload[0]);
		app_pdg_send_msg(grp, opcode, 0, 0);
	}
	else if (opcode == VANET_OP_ACCELEROMETER_RESET)
	{
		app_accel_init_dev();												// reset all registers
		app_accel_set_movement_threshold(bsp_cp_get_field(accel_thresh));	// reset state machine
		app_pdg_send_msg(grp, opcode, 0, 0);
	}
}

static const struct { uint16_t key; void (*handler)(uint8_t,uint8_t,const uint8_t*,uint16_t); } s_cmds[] =
{
    { CMDKEY(VANET_GRP_GENERAL, VANET_OP_API_VERSION), cmd_apiversion },
    { CMDKEY(VANET_GRP_GENERAL, VANET_OP_SET_BUZZER_START), cmd_buzzer },
    { CMDKEY(VANET_GRP_GENERAL, VANET_OP_SET_BUZZER_STOP), cmd_buzzer },
    { CMDKEY(VANET_GRP_GPS, VANET_OP_GET_GPS_STATE), app_gps_cmd_get_state },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_QUERY), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_SET_MOVEMENT_THRESHOLD), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_SET_BANDWIDTH), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_SET_RANGE), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_REGISTER_READ), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_REGISTER_WRITE), cmd_accelerometer },
	{ CMDKEY(VANET_GRP_ACCEL, VANET_OP_ACCELEROMETER_RESET), cmd_accelerometer },
    { 0, 0 }
};

void app_pdg_cmd_init(void)
{
    s_cmd_bytes_rx = 0;
    s_cmd_payload_len = 0;
}

void app_pdg_send_msg(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
    uint8_t hdr[4];
    hdr[0] = grp;
    hdr[1] = opcode;
    hdr[2] = (uint8_t) ((payload_len >> 8) & 0xff);
    hdr[3] = (uint8_t) (payload_len & 0xff);
    bsp_mux_send(VANET_MUXCH_UNIFIED, hdr, 4);
    if (payload) bsp_mux_send(VANET_MUXCH_UNIFIED, payload, payload_len);
}

static void exec_cmd(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
    int i;
    char msg[32];
    uint16_t key = CMDKEY(grp,opcode);
    
    bsp_logcat_printf(BSP_LOGCAT_INFO, "PDG Cmd %d:%d len=%d", grp, opcode, payload_len);
    
    for (i=0; s_cmds[i].handler; i++)
    {
        if (s_cmds[i].key == key)
        {
            s_cmds[i].handler(grp, opcode, payload,payload_len);
            return;
        }
    }
    
    dlib_snprintf(msg, 32, "Unknown cmd: %d %d", grp, opcode);
    app_pdg_send_msg(VANET_GRP_ERROR, VANET_OP_ERROR, (const uint8_t*) msg, strlen(msg)+1);
}

void app_pdg_cmd_put(uint8_t b)
{
    s_cmdbuf[s_cmd_bytes_rx++] = b;
        
    if (s_cmd_bytes_rx == 4)
    {
        s_cmd_payload_len = ((s_cmdbuf[2] & 0xff) << 8) | (s_cmdbuf[3] & 0xff);
        if (s_cmd_payload_len > (sizeof(s_cmdbuf) - 4))
        {
            s_cmd_payload_len = sizeof(s_cmdbuf) - 4;
        }
    }
    if (s_cmd_bytes_rx >= (4 + s_cmd_payload_len))
    {
        exec_cmd(s_cmdbuf[0], s_cmdbuf[1], s_cmdbuf  + 4, s_cmd_payload_len);
        s_cmd_bytes_rx = 0;
        s_cmd_payload_len = 0;
    }
}
