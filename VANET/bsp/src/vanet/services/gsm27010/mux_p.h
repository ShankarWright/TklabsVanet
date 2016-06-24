/**
 *	@file	mux_p.h
 *
 *	@brief	Internal (Private) API for GSM 27.010 Basic Frame Multiplexer
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

#ifndef _MUX_P_H
#define _MUX_P_H

/// Source BSP_TKVS_SRC_MUX_INTERNAL Events
enum
{
	BSP_MUX_EVENT_UART_RX			= 0x1000,	///< Data available in our circular buffer to be de-mux'd
	BSP_MUX_EVENT_UART_TX			= 0x2000,	///< Data available in our circular buffer to be sent
	BSP_MUX_EVENT_DLCI_SEND			= 0x4000,	///< Data to be mux'd and sent
};

/// Determines open mode for MUX DLCI channels
typedef enum openMode
{
	MUXOPEN_TE_ACTIVE,     ///< Actively initiate connection (i.e. Terminal Equipment - TE/AP)
	MUXOPEN_UE_PASSIVE,    ///< Passively wait for connection (i.e. User Equipment - UE/BP)
	__INVALID_MUXOPEN      ///< Invalid Open mode
} openMode;
  
/** Boundary Flags */
typedef enum boundary_flags
{
	BOUND_BASIC = 0xf9, /**< Basic Option Boundary Flag */
	BOUND_ADV = 0x7e    /**< Advanced Option Boundary Flag */
} boundary_flags;

/** Frame Types */
typedef enum frame_types
{
	FRAME_SABM = 0x2f,  /**< (SABM) Set Asyncronous Balanced Mode */
	FRAME_UA = 0x63,    /**< (UA) Unnumbered Acknowlegement */
	FRAME_DM = 0x0f,    /**< (DM) Disconnected Mode */
	FRAME_DISC = 0x43,  /**< (DISC) Disconnect */
	FRAME_UIH = 0xef,   /**< (UIH) Unnumbered Information w/ Header Check */
	FRAME_UI = 0x03     /**< (UI) Unnumbered Information */
} frame_types;

/** Control Bits */
typedef enum control_bits
{
	EA_BIT = 0x01,      /**< Extended Address Bit (1 = Last Octet) */
	PF_BIT = 0x10,      /**< Poll / Final Bit */
	CR_BIT = 0x02       /**< Command / Response Bit */
} control_bits;

/** Control Channel (DLCI 0) Commands */
typedef enum control_channel_commands
{
	CTRL_CLD = 0xC0     /**< Multiplexor Close Down (5.4.6.3.3) */
} control_channel_commands;

/** Frame Direction */
typedef enum frame_type
{
	FRAME_TX,           /**< Frame Transmitted to MUX */
	FRAME_RX,           /**< Frame Received from MUX */
	FRAME_BAD,          /**< Frame Bad */
	FRAME_DROP          /**< Frame Dropped */
} frame_type;

/** Frame Status (Internal) */
typedef enum frame_status
{
	FRAME_VALID = 0x00,         /**< Frame Valid */
	FRAME_INVALID_FCS = 0x01    /**< Invalid FCS */
} frame_status;

/** Mux Channel State (Internal) */
typedef enum mux_chan_state
{
	MUX_CLOSED,
	MUX_OPENED,
	MUX_OPEN_PENDING,
	MUX_CLOSE_PENDING
} mux_chan_state;

/** State machine states for MUX Frame Decoder */
typedef enum mux_state_t
{
	OPEN_FLAG,      /**< Waiting for Flag (Boundary) Field */
	CONTROL_FIELD,  /**< Processing Control Field */
	ADDRESS_FIELD,  /**< Processing Address Field */
	LEN_BYTE1,      /**< Processing Length Byte 1 */
	LEN_BYTE2,      /**< Processing Length Byte 2 (if present) */
	INFORMATION,    /**< Processing Information Octets */
	FCS_FIELD,      /**< Processing FCS Field */
	CLOSE_FLAG      /**< Processing Flag (Boundary) Field */
} mux_state_t;

/**
    * MUX Frame
    *
    * @warning start to end must remain in order and size
    */
typedef struct mux_frame_t
{
    unsigned char   start;          /**< Frame flag field (start) */
    unsigned char   addr;           /**< Frame address field */
    unsigned char   control;        /**< Frame control field */
    unsigned char   len1;           /**< Length 1 field */
    unsigned char   len2;           /**< Length 2 field (if present) */
    unsigned char   rx_data[128];   /**< Information octets */
    unsigned char   fcs;            /**< FCS field */
    unsigned char   end;            /**< Frame flag field (end) */

    unsigned char   complete;       /**< Complete Flag, 1=Complete */
    unsigned char   status;         /**< Status Flag, 0=Frame Valid */
    int             info_len;       /**< Information field length */
    uint32_t        start_tick;     /**< The tick when the frame started */
        
    mux_state_t     mux_state;      /**< Parser state for this frame */
    int             data_len;       /**< Parser data length for this frame */
    int             dlci;           /**< Frame DLCI field */
} mux_frame_t;

/** Mux Channel (DLCI) State */
typedef struct mux_stat_t
{
    mux_chan_state opened;    /**< Mux Channel Opened Flag */
} mux_stat_t;
	
/// Write a frame to the mux
void mux_write_frame(int channel, int frame, uint8_t *inbuf, int len);

/// Read a frame from the mux
void mux_recv_data(bsp_circ_buffer_t *mux_buf);

/// Find a frame from within a QtklBuffer
int mux_find_frame(bsp_circ_buffer_t *buf, mux_frame_t *frame);

/// Check a mux frame for validity
void mux_check_frame(mux_frame_t *frame);

/// Calculate FCS (Frame Check Sequence)
uint8_t mux_calc_fcs(const uint8_t *inbuf, int len, uint8_t start_fcs);

/// Check FCS (Frame Check Sequence)
bool mux_check_fcs(unsigned char calc_fcs, unsigned char fcs);

/// Dump frame
void dump_frame(frame_type dir, const mux_frame_t *);

// Module Variables
extern mux_frame_t partial_frame;
extern mux_stat_t mux_stat[16];
extern uint8_t s_termios_mux;

#endif // _MUX_P_H