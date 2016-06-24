/**
 *	@file	bsp_init.h
 *
 *	@brief	BSP Initialization
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013-2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef _BSP_INIT_H
#define _BSP_INIT_H

/**
 * Initialize the BSP Hardware
 *
 * @warning     This should be the FIRST thing that happens after _stext
 */
extern void bsp_init(void);

#endif // _BSP_INIT_H