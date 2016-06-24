/**
 *	@file	codeplug.h
 *
 *	@brief	Codeplug
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

#ifndef _CODEPLUG_H
#define _CODEPLUG_H

/**
 * @defgroup group_bsp_axamio_services_codeplug Codeplug
 *
 * @{
 */

#define BSP_CP_SIGNATURE_1	0xba5eba11
#define BSP_CP_SIGNATURE	0xdecafbad

// Used in conf_codeplug.h to define application fields for debug printing
#define DEFINE_APP_CP_FIELD(_field, _type, _desc) { #_field, \
                                                    STRUCT_OFFSET(user_codeplug_t, _field), \
                                                    STRUCT_SIZE(user_codeplug_t, _field), \
                                                    _type, \
                                                    true, \
                                                    _desc }


/* Application / User Codeplug */
//#include "conf_codeplug.h"

/**
 *  @name Codeplug
 *
 *  Manage non-volatile memory
 *
 *  @{
 */

#include "defaults.h"

/// Combined BSP and User (App) Codeplugs - the App must define user_codeplug_t
typedef const struct 
{
    bsp_codeplug_t  bsp_codeplug;
    //user_codeplug_t user_codeplug;
} codeplug_t;
    
extern bsp_codeplug_t *p_bsp_codeplug;
//extern user_codeplug_t *p_user_codeplug;

/** Read a codeplug field

    Ex. cp_get_field(serial)[0] == device_type
 */
#define bsp_cp_get_field(field)             p_bsp_codeplug->field

/// Write a codeplug field
#define bsp_cp_set_field(field, val)        bsp_cp_write((void *)&p_bsp_codeplug->field,&val,sizeof(p_bsp_codeplug->field))

#define app_cp_get_field(field)             p_user_codeplug->field
#define app_cp_set_field(field, val)        bsp_cp_write((void *)&p_user_codeplug->field,&val,sizeof(p_user_codeplug->field))

/// Write data into the codeplug.  Use cp_set_field() instead
extern void bsp_cp_write(void* dst, const void* src, size_t size);

/// Returns the validity of the codeplug
extern bool bsp_cp_valid(void);

/// Initialize the codeplug
extern void bsp_cp_init(void);

/// Make sure the codeplug is valid
extern bool bsp_cp_verify(void);

/// Make sure the codeplug is the latest version
extern bool bsp_cp_verify_version(void);

/// Check if an address is in user page of flash
static inline bool bsp_util_address_in_user_page(const void* dst)
{
	return ( ((uint8_t *)dst >= AVR32_FLASHC_USER_PAGE)
	&& (((uint8_t *)dst) <= (AVR32_FLASHC_USER_PAGE + AVR32_FLASHC_USER_PAGE_SIZE)) );
}

/// @}

/// @}

#endif  // _CODEPLUG_H
