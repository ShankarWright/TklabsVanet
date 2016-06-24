/**
 *	@file	mux.h
 *
 *	@brief	GSM 27.010 Basic Frame Multiplexer
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

#ifndef _MUX_H
#define _MUX_H

/// Initialize the mux
void bsp_mux_init(void);

/// Idle loop to grab data from ISR (OS too slow!)
void bsp_mux_idle_loop(void);

/// Send Data via Mux
void bsp_mux_send(uint8_t dlci, const void* data, uint16_t data_length);

/// Maximum number of MUX channels supported (Including DLCI 0 !)
#define BSP_TKVS_SRC_MUX_DLCI_NUM	6

/// Mux Echo Channel
#define BSP_TKVS_MUX_ECHO_DLCI		(BSP_TKVS_SRC_MUX_DLCI_NUM - 1)

/// Source BSP_TKVS_SRC_MUX_DLCI* Events
enum
{
	// these events are published per Mux channel
	BSP_MUX_EVENT_CONNECT			= 0x0001,   ///< Channel connected
	BSP_MUX_EVENT_DISCONNECT		= 0x0002,   ///< Channel disconnected
	BSP_MUX_EVENT_DATA_RCVD			= 0x0004,   ///< Channel data received
};

/// Map Mux DCLI to TKVS Source
#define BSP_MUX_DLCI_TO_TKVS_SOURCE(_dlci)		(BSP_TKVS_SRC_MUX_DLCI_START + _dlci)

/// Map TKVS Source to Mux DLCI
#define BSP_TKVS_SOURCE_TO_MUX_DLCI(_source)	(_source - BSP_TKVS_SRC_MUX_DLCI_START)

#endif // _MUX_H