/**
 *	@file	conf_buffers.h
 *
 *	@brief	Configure dynamic memory pools
 *
 *  The intent of this configuration file is like the other ASF configuration
 *  files like conf_clock.h, etc.
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

#ifndef CONF_BUFFERS_H
#define CONF_BUFFERS_H

// Whether or not to do boundary checks.  Adds 8 byte overhead per buffer
#define CONFIG_BSP_BUFFERS_BOUNDARY_CHECK

// The number of memory pools
#define CONFIG_BSP_BUFFERS_NUM_POOLS            3

// Pool 0: 64 x 256 = 4k
#define CONFIG_BSP_BUFFERS_POOL0_BUFSIZE        64
#define CONFIG_BSP_BUFFERS_POOL0_COUNT          64

// Pool 1: 512 x 8 = 16k
#define CONFIG_BSP_BUFFERS_POOL1_BUFSIZE        512
#define CONFIG_BSP_BUFFERS_POOL1_COUNT          32

// Pool 2: 1500 x 4 = 6k
#define CONFIG_BSP_BUFFERS_POOL2_BUFSIZE        1500
#define CONFIG_BSP_BUFFERS_POOL2_COUNT          4

#endif /* CONF_BUFFERS_H */