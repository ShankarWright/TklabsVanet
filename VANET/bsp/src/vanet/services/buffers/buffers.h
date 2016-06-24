/**
 *	@file	buffers.h
 *
 *	@brief	Dynamic memory allocation
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

#ifndef BSP_BUFFERS_H
#define BSP_BUFFERS_H

/**
 * @defgroup group_bsp_services_buffers Dynamic memory allocation
 *
 * Pool-based dynamic memory allocation
 *
 * @{
 */

/**  
 *  @name Initialization
 *  @{
 */
/// Initialize service
extern void bsp_buffers_init(void);
/// @}

/**
 *  @name Public API
 *  @{
 */

/// A malloc routine for the BSP
extern void* bsp_malloc(size_t bytes);

/// A free routine for the BSP
extern void bsp_free(void* ptr);

/// Dump buffer state
extern void bsp_buffer_dump(uint8_t port);


/// @}

/// @}

#endif // BSP_LOGCAT_H