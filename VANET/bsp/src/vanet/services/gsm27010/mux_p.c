/**
 *	@file	mux_p.c
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

#include <string.h>
#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_MUX

#include "mux_p.h"

#define D(msg)                                  bsp_logcat_print(BSP_LOGCAT_MUX, msg)
#define DF(msg, ...)                            bsp_logcat_printf(BSP_LOGCAT_MUX, msg, __VA_ARGS__)
#define DD(buf,len)                             bsp_logcat_dump(BSP_LOGCAT_MUX, buf, len)

#undef MUX_DEBUG
#ifdef MUX_DEBUG
#define DBG(msg)                                D(msg)
#define DBGF(msg, ...)                          DF(msg, __VA_ARGS__)
#define DBGD(buf,len)                           DD(buf,len)
#else
#define DBG(msg)
#define DBGF(msg, ...)
#define DBGD(buf,len)
#endif

#define FRAME_TIMEOUT_TICKS                     (2 * CONFIG_BSP_RTC_TICK_HZ) // 2 seconds

/// Reverse CRC Table lookup for MUX
const unsigned char reverse_crc_table[256] = {
	0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
	0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
	0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
	0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
	0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
	0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
	0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
	0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
	0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
	0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
	0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
	0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
	0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
	0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
	0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
	0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
	0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
	0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
	0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
	0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
	0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
	0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
	0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
	0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
	0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
	0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
	0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
	0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
	0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
	0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
	0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
	0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

void mux_recv_data(bsp_circ_buffer_t *mux_buf)
{
	int bytes_checked;
	int len = bsp_circ_size(mux_buf);
	uint8_t frame_type, dlci;
	
	while (len > 0)
	{
		DBGF("mux_recv_data len=%d", len);
		
		bytes_checked = mux_find_frame(mux_buf, &partial_frame);
		
		if (partial_frame.complete)
		{
			DBG("Got a Frame!");
			mux_check_frame(&partial_frame);
			if (partial_frame.status == FRAME_VALID)
			{
				DBG("And it was valid!");
				frame_type = partial_frame.control & ~PF_BIT;
				dlci = partial_frame.addr >> 2;
				
				// decode frame
				dump_frame(FRAME_RX, &partial_frame);
				
				// Supervisory Frames
				switch(frame_type)
				{
					case FRAME_SABM:
					    if (dlci < BSP_TKVS_SRC_MUX_DLCI_NUM && 
							bsp_tkvs_is_subscribed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_TKVS_ALL_EVENTS) &&
							(dlci == 0 || mux_stat[0].opened == MUX_OPENED))
						{
							mux_write_frame(dlci, FRAME_UA | PF_BIT, NULL, 0);
							mux_stat[dlci].opened = MUX_OPENED;
							bsp_tkvs_publish_immed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_MUX_EVENT_CONNECT, 0);
						}
						else
						{
							mux_write_frame(dlci, FRAME_DM | PF_BIT, NULL, 0);
							mux_stat[dlci].opened = MUX_CLOSED;
						}
						break;
					case FRAME_DISC:
						if (dlci < BSP_TKVS_SRC_MUX_DLCI_NUM &&
							bsp_tkvs_is_subscribed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_TKVS_ALL_EVENTS))
						{
							mux_write_frame(dlci, FRAME_UA | PF_BIT, NULL, 0);
							bsp_tkvs_publish_immed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_MUX_EVENT_DISCONNECT, 0);
						}							
						else
						{
							mux_write_frame(dlci, FRAME_DM | PF_BIT, NULL, 0);
						}						
						mux_stat[dlci].opened = MUX_CLOSED;
						break;
					case FRAME_UIH:
						if (dlci < BSP_TKVS_SRC_MUX_DLCI_NUM && 
							bsp_tkvs_is_subscribed(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_TKVS_ALL_EVENTS) &&
							mux_stat[dlci].opened == MUX_OPENED)
						{
							bsp_tkvs_publish_data(BSP_MUX_DLCI_TO_TKVS_SOURCE(dlci), BSP_MUX_EVENT_DATA_RCVD,
												  partial_frame.rx_data, partial_frame.len1 >> 1);
						}
						else
						{
							mux_write_frame(dlci, FRAME_DM | PF_BIT, NULL, 0);
						}							
				}
			}
            else
            {
                DF("Frame check mismatch: %x %x %x %d %d %d %d",
                   partial_frame.fcs, partial_frame.complete, partial_frame.status, partial_frame.len1,
                   partial_frame.info_len, partial_frame.data_len, partial_frame.dlci);
            }                    
			
			// clean up for start of next frame
			memset(&partial_frame, 0, sizeof(partial_frame));
		}
		
		len = len - bytes_checked;			
	}
}

void mux_write_frame(int channel, int frame, uint8_t *inbuf, int len)
{
	int frame_start_len = 4;
	mux_frame_t mux_frame;
	
	// Debug
	memset(&mux_frame, 0xaa, sizeof(mux_frame_t));

	// Flag, Addr, Control
	mux_frame.start = BOUND_BASIC;
	mux_frame.addr = ((channel & 0x3f) << 2) | EA_BIT | CR_BIT;
	mux_frame.control = frame;

	// Length
	mux_frame.info_len = len;
	if (len > 127)
	{
		frame_start_len = 5;
		mux_frame.len1 = ((0x7f & len) << 1);
		mux_frame.len2 = (0x7f80 & len) >> 7;  // Forces EA_BIT=0
	}
	else
	{
		mux_frame.len1 = EA_BIT | (len << 1);
		mux_frame.len2 = 0;
	}

	// Copy the info
	memcpy(mux_frame.rx_data, inbuf, len);

	// FCS - Skip the Boundary Flag
	mux_frame.fcs = 0xff - mux_calc_fcs(&mux_frame.addr, frame_start_len - 1, 0xff);

	// End Flag
	mux_frame.end = BOUND_BASIC;

	// Dump Frame
	dump_frame(FRAME_TX, &mux_frame);

	// Write Frame
	frame_start_len = 4;           // Frame Start Length
	if (mux_frame.info_len > 127)
	{
		frame_start_len = 5;
	}
	
	bsp_termios_write(s_termios_mux, &mux_frame.start, frame_start_len);
	bsp_termios_write(s_termios_mux, mux_frame.rx_data, mux_frame.info_len);
	bsp_termios_write(s_termios_mux, &mux_frame.fcs, 2);						// fcs + end
	bsp_termios_flush(s_termios_mux);
}

// "statics"
uint8_t find_buf[128];
mux_state_t mux_state = OPEN_FLAG;
int data_len = 0;

int mux_find_frame(bsp_circ_buffer_t *buf, mux_frame_t *frame)
{
	int bytes_processed = 0;
	int bytes_peeked;
	int len = bsp_circ_size(buf);
	uint8_t *dptr = find_buf;
	
	DBGF("mux_find_frame state=%d len=%d", mux_state, len);
    
	bytes_peeked = bsp_circ_peek(buf, bsp_circ_begin(buf), find_buf, len);
	DBGF("bytes_peeked=%d", bytes_peeked);
    DBGD(find_buf, bytes_peeked);
	
	while (bytes_processed < len && frame->complete == 0)
	{
		//print_dbg("mux_state="); print_dbg_int(mux_state); print_dbg("\r\n");
        
        if (mux_state > OPEN_FLAG && (bsp_rtc_get_ticks() - frame->start_tick) > FRAME_TIMEOUT_TICKS)
        {
            D("FRAME TIMEOUT");
            memset(frame, 0, sizeof(mux_frame_t));
            mux_state = OPEN_FLAG;
        }
        
		switch (mux_state)
		{
			case OPEN_FLAG:
				if (*dptr == BOUND_BASIC)
				{
					frame->start = *dptr;
                    frame->start_tick = bsp_rtc_get_ticks();
					mux_state = ADDRESS_FIELD;
				}
				break;
			case ADDRESS_FIELD:
                // don't accept another F9 as a dlci.  this case comes up if we got out of sync
                // and the previous F9 was actually a close frame
                if (*dptr != BOUND_BASIC)
                {
				    frame->addr = *dptr;
				    mux_state = CONTROL_FIELD;
                }                
				break;
			case CONTROL_FIELD:
				frame->control = *dptr;
				mux_state = LEN_BYTE1;
				break;
			case LEN_BYTE1:
				frame->len1 = *dptr;
				frame->len2 = 0;
				if (frame->len1 & EA_BIT)
				{
					frame->info_len = frame->len1 >> 1;
					if (frame->info_len == 0)
						mux_state = FCS_FIELD;
					else
						mux_state = INFORMATION;	
				}
				else
				{
					mux_state = LEN_BYTE2;
				}
				break;
			case LEN_BYTE2:
				frame->len2 = *dptr;
				frame->info_len = (frame->len1 >> 1);
				/* FIX ME!? */
				break;
			case INFORMATION:
				frame->rx_data[data_len++] = *dptr;
				if (data_len == frame->info_len)
				{
					data_len = 0;
					mux_state = FCS_FIELD;
				}
				break;
			case FCS_FIELD:
				frame->fcs = *dptr;
				mux_state = CLOSE_FLAG;
				break;
			case CLOSE_FLAG:
				if (*dptr == BOUND_BASIC)
				{
					frame->complete = 1;
				}
				mux_state = OPEN_FLAG;
				break;
			default:
				break;
		}
		bytes_processed++;
		dptr++;
	}
	
	// now actually dequeue the # of bytes we have already processed above
	bsp_circ_read(buf, find_buf, bytes_processed);
	
	return bytes_processed;
}

void mux_check_frame(mux_frame_t *frame)
{
	uint8_t fcs;
	bool fcs_valid;
	int len;
	
	if (frame->len2 != 0)
		len = 4;
	else
		len = 3;
		
	fcs = mux_calc_fcs(&frame->addr, len, 0xff);
	
	fcs_valid = mux_check_fcs(fcs, frame->fcs);
	
	if (!fcs_valid)
		frame->status |= FRAME_INVALID_FCS;
}

bool mux_check_fcs(unsigned char calc_fcs, unsigned char fcs)
{
	uint8_t check_fcs = reverse_crc_table[calc_fcs ^ fcs];
	
	return (check_fcs == 0xcf) ? true : false;
}

uint8_t mux_calc_fcs(const uint8_t *inbuf, int len, uint8_t start_fcs)
{
	uint8_t fcs = start_fcs;
	
	for (int i=0; i<len; i++)
	{
		fcs = reverse_crc_table[fcs ^ inbuf[i]];
	}
	return fcs;
}

void dump_frame(frame_type dir, const mux_frame_t *frame)
{
    static char tmp[16];
    static char out[128];

	out[0] = '\0';
	int len = 0;
	
	if (dir == FRAME_TX)
		strcat(out, " | TX |  ");
	else if (dir == FRAME_RX)
		strcat(out, " | RX |  ");
	else if (dir == FRAME_DROP)
		strcat(out, " |DROP|  ");
	else
		strcat(out, " | ?? |  ");
		
	if (frame->start == BOUND_BASIC)
	{
		 // decode address field (dlci, ea, c/r bits)
		 dlib_snprintf(tmp, sizeof(tmp), "dlci %d ", frame->addr >> 2);
		 strcat(out, tmp);

		 if (frame->addr & 0x03)
		 {
			 strcat(out, "( ");
			 if (frame->addr & EA_BIT) strcat(out, "ea ");
			 if (frame->addr & CR_BIT) strcat(out, "c/r ");
			 strcat(out, ") ");
		 }

		 // decode control field
		 switch (frame->control & ~PF_BIT)
		 {
			 case FRAME_SABM:
				strcat(out, "SABM ");
				break;
			 case FRAME_UA:
				strcat(out, "UA ");
				break;
			 case FRAME_DM:
				strcat(out, "DM ");
				break;
			 case FRAME_UIH:
				strcat(out, "UIH ");
				break;
			 case FRAME_UI:
				strcat(out, "UH ");
				break;
			 case FRAME_DISC:
				strcat(out, "DISC ");
				break;
			 default:
				 strcat(out, "??? ");
				break;
		 }
		 
		 if (frame->control & PF_BIT)
		 {
			 strcat(out, "( pf ) ");
		 }

		 // decode length field
		 if (frame->len1 & EA_BIT)
		 {
			 len = frame->len1 >> 1;
		 }
		 //else
		 //{
	     //    #error "Can't handle";
		 //}
		 dlib_snprintf(tmp, sizeof(tmp), "len %d ", len);
		 strcat(out, tmp);

		 // decode fcs field
		 dlib_snprintf(tmp, sizeof(tmp), "fsc %02x ", frame->fcs);
		 strcat(out, tmp);

		 // display information if any was present
		 if (len > 0)
		 {
			DBGD(frame->rx_data, len);
		 }
	}
	else
	{
		D("Unsupported framing");
	}		
	
	D(out);
}

#endif // CONFIG_BSP_ENABLE_MUX