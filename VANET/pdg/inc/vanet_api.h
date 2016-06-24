/**
 *  @file   vanet_api.h
 *
 *  @brief  VANET Mainboard to Daughterboard API
 *
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

#ifndef VANET_API_H
#define VANET_API_H

#include <stdint.h>

// Mux channel definitions
#define VANET_MUXCH_TIMESYNC                1
#define VANET_MUXCH_UNIFIED                 2
#define VANET_MUXCH_ACCELEROMETER_RAW       3
#define VANET_MUXCH_GPS_RAW                 4
#define VANET_MUXCH_ECHO                    5

#define VANET_MUXCH_MAX                     6

// The API version
#define VANET_API_VERSION                   4

/*
    This is the main payload structure for commands sent on the unified mux channel (2). All fields
    are big-endian (network order)

      +-- Group (8 bits)
      |   +-- Opcode (8 bits)
      |   |   +-- Payoad length (16 bits)
      |   |   |       +-- Payload ...
      |   |   |       |
    | 0 | 1 | 2 | 3 | 4 | ...

    Messages are either "Commands" sent from the main board to daughterboard (Group < 0x80) or
    asynchronous "Events" from the daughterboard to the main board (Group >= 0x80).  The
    daughterboard responds to Commands with either the same Group and Opcode sent from the
    mainboard or an error message (Group = Opcode = 0). The daughterboard may emit Event messages
    at any time.
*/

typedef struct
{
    uint8_t     group;
    uint8_t     opcode;
    uint16_t    length;
} vanet_api_hdr_t;


#define VANET_MAIN_TO_DAUGHTER  0x00
#define VANET_DAUGHTER_TO_MAIN  0x80

/*
    The set of command groups
*/
enum
{
    VANET_GRP_ERROR = 0,
    VANET_GRP_GENERAL,
    VANET_GRP_ACCEL,
    VANET_GRP_GPS,

    VANET_GRP_ACCEL_EVENT = 0x80,
    VANET_GRP_GPS_EVENT,
    VANET_GRP_BUTTON_EVENT,
};


/*******************************************************************

    Command Group:  Error

*******************************************************************/
enum
{
    VANET_OP_ERROR = 0
};

/*
    Opcode: Error

    Command Payload:
        N/A

    Response Payload:
        Byte 0      - Null-terminated error message

*/

/*******************************************************************

    Command Group:  General

*******************************************************************/
enum
{
    VANET_OP_API_VERSION = 0,       //
    VANET_OP_SET_BUZZER_START,
    VANET_OP_SET_BUZZER_STOP,
};

/*
    Opcode: API Version

    Command Payload:
        Empty

    Response Payload:
        Byte 0      - The API version
*/

/*
    Opcode: Buzzer Start

    Command Payload:
        Byte 0      - Number of states (max 16)
        Byte 1      - Number of times to repeat the cycle of states (0 for infinite)
        Byte 2-3    - State 1 tone frequency (0 for off)
        Byte 4-5    - State 1 duration in ms (0 for infinite)
        ...

    Response Payload:
        Empty
*/

/*
    Opcode: Buzzer Stop

    Command Payload:
        Empty

    Response Payload:
        Empty
*/

/*******************************************************************

    Command Group:  Accelerometer

*******************************************************************/
enum
{
	VANET_OP_ACCELEROMETER_RESET,
	VANET_OP_ACCELEROMETER_QUERY,
	VANET_OP_ACCELEROMETER_SET_MOVEMENT_THRESHOLD,
	VANET_OP_ACCELEROMETER_SET_BANDWIDTH,
	VANET_OP_ACCELEROMETER_SET_RANGE,
	VANET_OP_ACCELEROMETER_REGISTER_READ,
	VANET_OP_ACCELEROMETER_REGISTER_WRITE,
};

/*
    Opcode: Reset
	
	Command Payload:
	    Empty
		
	Response Payload:
	    Empty
*/

/*
    Opcode: Query

    Command Payload:
        Empty

    Response Payload:
        Byte 0-1		- Accelerometer X, Signed
        Byte 2-3		- Accelerometer Y, Signed
        Byte 4-6		- Accelerometer Z, Signed
        Byte 6			- Temperature, Signed
        Byte 7          - Range, Unsigned, +/- G Scale (e.g. 2, 4, 8, 16, etc.)
		Byte 8-9        - Bandwidth, Unsigned (e.g. 100Hz)
		Byte 10-11      - Threshold, Unsigned, G*1000
*/

typedef struct
{
	int16_t		x_accel;
	int16_t		y_accel;
	int16_t		z_accel;
	int8_t		temperature;
	uint8_t		range;
	uint16_t	bandwidth;
	uint16_t	threshold;
} vanet_api_accel_t;

/*
	Opcode: Set Movement Threshold
	
	Command Payload:
		Byte 0,1		- Threshold, In G * 1000, unsigned
						  e.g. 0 = disabled
						       1000 = 1G
							   16000 = 16G
					      Note: Send 0 to disable built in algorithm
						        Accelerometer Range will be adjusted to accommodate requested threshold
						  
	Response Payload:
		Empty
*/

/*
	Opcode: Set Bandwidth
	
	Command Payload:
		Byte 0,1		- Bandwidth, In Hz, Unsigned
						  
	Response Payload:
		Empty
*/

/*
	Opcode: Set Range
	
	Command Payload:
		Byte 0			- Range, Unsigned, +/- G
						  
	Response Payload:
		Empty
*/

/*
	Opcode: Read Register
	
	Command Payload:
		Byte 0			- Register Number
						  
	Response Payload:
		Byte 0			- Register Value
*/

/*
	Opcode: Write Register
	
	Command Payload:
		Byte 0			- Register Number
		Byte 1			- Register Value
						  
	Response Payload:
		Empty
*/

/*******************************************************************

    Command Group:  GPS

*******************************************************************/
enum
{
    VANET_OP_GET_GPS_STATE = 0,
};

/*
    Opcode: Get GPS State

    Command Payload:
        Empty

    Response Payload:
        Byte 0-3    - State flags (see below)
        Byte 4-7    - Latitude in degrees * 10,000,000
        Byte 8-11   - Longitude in degrees * 10,000,000
        Byte 12-13  - Altitude in meters
        Byte 14-15  - Speed in knots
        Byte 16-17  - Track angle in degrees True
        Byte 18     - Number of tracked satellites in view
        Byte 19     - Horizontal dilution of precision * 10
        Byte 20-23  - Current time as Unix timestamp
        Byte 24-27  - Microsecond part of timestamp
*/

#define VANET_API_GPS_LOC_MULTIPLIER            10000000
#define VANET_API_GPS_LOC_MULTIPLIERF           10000000.0

enum
{
    VAPET_API_GPS_LOCATION_LOCK                 = 0x00000001,
    VAPET_API_GPS_TIME_LOCK                     = 0x00000002,
};

typedef struct
{
    uint32_t flags;
    int32_t latitude;
    int32_t longitude;
    int16_t altitude;
    uint16_t speed;
    int16_t direction;
    uint8_t satellites;
    uint8_t hdop;
    uint32_t timestamp;
    uint32_t timestamp_us;
} vanet_api_gps_state_t;

/*******************************************************************

    Command Group:  Accelerometer Events

*******************************************************************/
enum
{
    VANET_OP_MOTION_INTERRUPT = 0,
};

/*
    Opcode: Motion Interrupt

    Command Payload:
        Empty

    Response Payload:
        Empty
*/

/*******************************************************************

    Command Group:  GPS Events

*******************************************************************/

/*******************************************************************

    Command Group:  Button Events

*******************************************************************/

enum
{
    VANET_OP_BUTTON_PRESS = 0,
    VANET_OP_BUTTON_RELEASE
};

/*
    Opcode: Button Press

    Event Payload:
        Byte 0              - Button ID
*/

/*
    Opcode: Button Release

    Event Payload:
        Byte 0              - Button ID
*/

/*
    The time sync channel (1) is used exclusively for NTP-like time sync.  The main board sends a
    time request and the daughterboard replies with the current time.  Packet formats are as
    follows:

    Time Request:
        Byte 0              - 0x69

    Time Response:
        Byte 0-3            - Current time as Unix timestamp
        Byte 4-7            - Microsecond part of timestamp
*/
#define VANET_API_TIME_REQUEST          0x69

typedef struct
{
    uint32_t timestamp;
    uint32_t timestamp_us;
} vanet_api_timestamp_t;

#endif