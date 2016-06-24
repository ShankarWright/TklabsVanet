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

#ifndef PDG_CMD_H
#define PDG_CMD_H

//                                 group,  opcode, payload,       payload len
typedef void (*app_pdg_cmd_hndr_t)(uint8_t,uint8_t,const uint8_t*,uint16_t);

/// Initialize cmd parser
extern void app_pdg_cmd_init(void);

// Put a byte
extern void app_pdg_cmd_put(uint8_t b);

// Send a message to the main board
extern void app_pdg_send_msg(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len);

#endif