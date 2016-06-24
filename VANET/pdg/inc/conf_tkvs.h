/**
 *	@file	conf_tkvs.h
 *
 *	@brief	Application sources
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
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef CONF_TKVS_H_INCLUDED
#define CONF_TKVS_H_INCLUDED

/// @weakgroup group_bsp_services_tkvs
/// @{

enum 
{
    // Application Tasks
	APP_TKVS_GPS_TASK = BSP_TKVS_SRC_APP_START,
	APP_TKVS_PDG_TASK,
	APP_TKVS_ACCEL_TASK,
};

/// @}

#endif // CONF_TKVS_H_INCLUDED