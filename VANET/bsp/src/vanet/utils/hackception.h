/**
 *	@file	hackception.h
 *
 *	@brief	Hackery to give something like C++ exceptions in C code
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

#ifndef _HACKCEPTION_H
#define _HACKCEPTION_H

/**
 * @defgroup group_bsp_utils_exception C++ Like Exception Functions
 *
 * C++ Like Exception Functions
 *
 * @{
 */

/// Hidden exception state
typedef struct {
	bool thrown;			///< true once exception is "thrown"
	int code;				///< exception value
} bsp_exception_t;

/// Allocate an exception
#define _alloc_exception(e) bsp_exception_t e; e.thrown = false;

/// Begin of try/catch block
#define _try

/// Exception catcher
#define _catch(e) __##e##__: if (e.thrown)

/// Throw an exception
#define _throw(e, c) { \
e.thrown = true; e.code = c; goto __##e##__; \
}

/// @}
#endif // _HACKCEPTION_H