/**
 *	@file	user_board.h
 *
 *	@brief	Board-specific defines
 *
 *  Includes the appropriate board definition file
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

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include "conf_vanet.h"

// Available hardware configurations
#define HW_VANET_DAUGHTER_REVA			1
#define HW_VANET_DAUGHTER_REVB          2

#if HARDWARE == HW_VANET_DAUGHTER_REVA
    #include "hw_vanet_daughterboard_rev_a.h"
	#define  BSP_HARDWARE_STR   "VANET-DaughterP1"
#elif HARDWARE == HW_VANET_DAUGHTER_REVB
	#include "hw_vanet_daughterboard_rev_a.h"
	#define  BSP_HARDWARE_STR   "VANET-DaughterP2"
#else
    #error "Unknown / Unsupported Hardware Configuration"
#endif    

#endif // USER_BOARD_H
