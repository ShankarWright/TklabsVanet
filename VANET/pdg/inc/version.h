/**
 *	@file	version.h
 *
 *	@brief	PDG Version Information
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
 *  (C) Copyright 2013-14 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef VERSION_H
#define VERSION_H

/// The software version string
#define VERSION_SOFTWARE                    "0.2"

/// The software version build number "X.X (Build N)"
#define VERSION_SOFTWARE_BUILD              10

/// The build date
#define VERSION_BUILD_DATE                  __DATE__ " " __TIME__

/// The compiler version
#define VERSION_COMPILER                    "AVR32 GCC " __VERSION__

/// The hardware revision string
#define VERSION_HARDWARE_STR                BSP_HARDWARE_STR

/// The product about message
#define VERSION_ABOUT                       "VANET Daughterboard " VERSION_SOFTWARE

#endif // VERSION_H
