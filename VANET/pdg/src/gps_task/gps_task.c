/**
 *	@file	gps_task.c
 *
 *	@brief	GPS Handler Task
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

#include <asf.h>
#include <string.h>
#include "vanet.h"
#include "vanet_api.h"
#include "pdg_cmd.h"

static OS_STK s_app_gps_task_stack[TASK_GPS_STACK_SIZE];
static void gps_task(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[16];
static vanet_api_gps_state_t s_gps_state;
static uint32_t s_gps_timestamp;
static int32_t s_gps_offset_us;
static uint8_t s_termios_gps;
static uint8_t s_termios_nmea;

static void gps_handler(int argc, char** argv, uint8_t port)
{
    if (argc > 1)
    {
        if (argc > 2 && !strcasecmp("stream", argv[1]))
        {
            if (!strcasecmp("on",argv[2]))
            {
                bsp_termios_printf(port, "NMEA stream enabled\r\n");
                s_termios_nmea = port;
            }
            else
            {
                bsp_termios_printf(port, "NMEA stream disabled\r\n");
                s_termios_nmea = 0xff;
            }
        }
    }
    else
    {
        bsp_termios_printf(port, "Flags: %X\r\n", s_gps_state.flags);
        bsp_termios_printf(port, "Location: %d, %d\r\n", s_gps_state.latitude, s_gps_state.longitude);
        bsp_termios_printf(port, "Altitude: %d m\r\n", s_gps_state.altitude);
        bsp_termios_printf(port, "Speed: %d knots, %d deg\r\n", s_gps_state.speed, s_gps_state.direction);
        bsp_termios_printf(port, "Satellites: %d\r\n", s_gps_state.satellites);
        bsp_termios_printf(port, "HDOP: %d\r\n", s_gps_state.hdop);
        bsp_termios_printf(port, "Offset: %d\r\n", s_gps_offset_us);
    }
}

static bsp_sti_command_t gps_command =
{
    .name = "gps",
    .handler = &gps_handler,
    .minArgs = 0,
    .maxArgs = 2,
    STI_HELP("gps                                  Show state of GPS\r\n"
             "gps stream on|off                    Print NMEA stream to port")
};

static void gps_timepulse_handler(void)
{
	gpio_clear_pin_interrupt_flag(GPS_TIMEPULSE_PIN);
    gpio_toggle_pin(BSP_GPS_SYNC_LED);

    // pulse comes at the beginning of the second, so timestamp is previous second.
    // always set the clock to the previous second so the offset is the number of us
    // until the next AST tick
    bsp_rtc_set_clock(s_gps_timestamp);
    s_gps_offset_us = 1000000 - bsp_rtc_get_clock_us();
    s_gps_state.flags |= VAPET_API_GPS_TIME_LOCK;
}

void app_gps_task_init()
{
	INT8U perr;
	
	// Register the GPS Timepulse pin
	INTC_register_GPIO_interrupt(&gps_timepulse_handler, GPS_TIMEPULSE_PIN);
	gpio_enable_gpio_pin(GPS_TIMEPULSE_PIN);
	gpio_enable_pin_glitch_filter(GPS_TIMEPULSE_PIN);
	gpio_enable_pin_interrupt(GPS_TIMEPULSE_PIN, GPIO_RISING_EDGE);

	// debug console message
	print_dbg("Creating GPS Task\r\n");
	
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "GPS task: OSQCreate");
	
	// Create the task
	perr = OSTaskCreateExt(gps_task,
		(void *)0,
		(OS_STK *)&s_app_gps_task_stack[TASK_GPS_STACK_SIZE - 1],
		TASK_GPS_PRIO,
		TASK_GPS_PRIO,
		(OS_STK *)&s_app_gps_task_stack[0],
		TASK_GPS_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OS_CHECK_PERR(perr, "GPS task: OSTaskCreateExt");
	
	OSTaskNameSet(TASK_GPS_PRIO, (INT8U *)"GPS", &perr);
	OS_CHECK_PERR(perr, "GPS task: OSTaskNameSet");
	
	// This is our serial port connection
	if (bsp_termios_find_port("GPS", &s_termios_gps))
	{
		bsp_tkvs_subscribe(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_gps), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_GPS_PRIO);
	}
	else
	{
		print_dbg("Unable to find Termios Port - GPS will not work!\r\n");
	}
    
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_TIMESYNC), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_GPS_PRIO);
	bsp_tkvs_subscribe(BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_GPS_RAW), BSP_TKVS_ALL_EVENTS, s_msg_flag, TASK_GPS_PRIO);
    
    memset(&s_gps_state, 0, sizeof(s_gps_state));
    s_gps_timestamp = 0;
    s_gps_offset_us = 0;
    s_termios_nmea = 0xff;
    
    bsp_sti_register_command(&gps_command);
}

static uint32_t gps_parse_timestamp(const char* ds, const char* ts)
{
    // Ex: time=201426.000 date=280114 -> 20:14:26 Jan 28, 2014 UTC    
    uint32_t r = 0;
    if (strlen(ds) >= 6 && strlen(ts) >= 6)
    {
        struct calendar_date dt;
        dt.date = 10 * (ds[0]-'0') + (ds[1]-'0') - 1;
        dt.month = 10 * (ds[2]-'0') + (ds[3]-'0') - 1;
        dt.year = 2000 + 10 * (ds[4]-'0') + (ds[5]-'0');
        dt.hour = 10 * (ts[0]-'0') + (ts[1]-'0');
        dt.minute = 10 * (ts[2]-'0') + (ts[3]-'0');
        dt.second = 10 * (ts[4]-'0') + (ts[5]-'0');
        
        r = calendar_date_to_timestamp(&dt);
        
        //bsp_termios_printf(BSP_TERMIOS_RAW_DEBUG_PORT, "%d %d\r\n", r, s_gps_offset_us);
    }
    
    return r;
}

static int32_t gps_parse_location(const char* str, char compass)
{
    // Ex: 2606.94253,N    08009.91494,W
    float x = atof(str);
    int degrees =  x / 100;
    float minutes = x - 100 * degrees;
    
    if (compass == 'S' || compass == 'W')
    {
        degrees = -degrees;
        minutes = -minutes;
    }
    
    return (degrees + minutes / 60) * VANET_API_GPS_LOC_MULTIPLIER;
}

static void gps_parse_nmea(char* nmea)
{
    int num_fields;
    char* line;
    char* fields[16];
    
    // send the raw data out before we mangle it
	bsp_mux_send(VANET_MUXCH_GPS_RAW, nmea, strlen(nmea));
	bsp_mux_send(VANET_MUXCH_GPS_RAW, "\r\n", 2);
    if (s_termios_nmea != 0xff)
    {
        bsp_termios_printf(s_termios_nmea, "%s\r\n", nmea);
        bsp_termios_flush(s_termios_nmea);
    }
    
    line = bsp_util_strtrim(nmea);            
    num_fields = bsp_util_strsplit(line, ",", fields, 16);
            
    if (!strcmp(fields[0], "$GPRMC") && num_fields >= 10)
    {
        // Ex: $GPRMC,021357.00,A,2606.94253,N,08009.91494,W,0.045,,310513,,,A*6F
        //     0      1         2 3          4 5           6 7    8 9
        if (fields[2][0] == 'A')
        {            
            s_gps_state.speed = atoi(fields[7]);
            s_gps_state.direction = atoi(fields[8]);
            s_gps_timestamp = gps_parse_timestamp(fields[9],fields[1]);
        }
        else
        {
            s_gps_state.flags &= ~VAPET_API_GPS_TIME_LOCK;
        }
    }
    else if (!strcmp(fields[0], "$GPGGA") && num_fields >= 10)
    {
        // Ex: $GPGGA,021357.00,2606.94253,N,08009.91494,W,1,03,2.27,1.6,M,-26.8,M,,*65
        //     0      1         2          3 4           5 6 7  8    9   10 11                
        bool gotit = false;
        if (fields[6][0] != '0')
        {
            s_gps_state.latitude = gps_parse_location(fields[2], fields[3][0]);
            s_gps_state.longitude = gps_parse_location(fields[4], fields[5][0]);
            s_gps_state.altitude = atoi(fields[9]);
            gotit = s_gps_state.latitude >= -900000000L && s_gps_state.latitude <= 900000000L &&
                    s_gps_state.longitude >= -1800000000L && s_gps_state.longitude <= 1800000000L;
        }
        
        if (gotit)
        {
            s_gps_state.flags |= VAPET_API_GPS_LOCATION_LOCK;
        }
        else
        {
            s_gps_state.flags &= ~VAPET_API_GPS_LOCATION_LOCK;
        }
    }
    else if (!strcmp(fields[0], "$GPGSA") && num_fields >= 15)
    {
        // Ex: $GPGSA,A,2,17,04,10, , , , , , , , , ,2.48,2.27,1.00*08
        //     0      1 2 3  4  5  6 7 8 9 0 1 2 3 4 5    6    7
        float pdop = atof(fields[15]);
        if (pdop < 25)
        {
            s_gps_state.hdop = (uint8_t) (pdop * 10);
        }
    }
    else if (!strcmp(fields[0], "$GPGSV") && num_fields >= 3)
    {
        // Ex: $GPGSV,1,1,03,04,65,276,37,10,36,171,30,17,52,025,34*4B
        //     0      1 2 3
        s_gps_state.satellites = atoi(fields[3]);
    }    
}

void app_gps_cmd_get_state(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len)
{
    // we already store the GPS state in this format, so just send it
    s_gps_state.timestamp = bsp_rtc_get_clock();
    s_gps_state.timestamp_us = bsp_rtc_get_clock_us() + s_gps_offset_us;
    if (s_gps_state.timestamp_us > 1000000)
    {
        s_gps_state.timestamp++;
        s_gps_state.timestamp_us -= 1000000;
    }
    app_pdg_send_msg(grp,opcode,(const uint8_t*) &s_gps_state,28);
}

static void gps_task(void *p_arg)
{
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	
	(void) p_arg;
	
	// Task Loop
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);    // block forever on my queue
		if (msg != (void *)0)
		{
			if (msg->source == BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_gps))
			{
				if (msg->event == BSP_TERMIOS_INPUT_READY)
				{
					// Need to do anything?
				}
				else if (msg->event == BSP_TERMIOS_INPUT_RX)
				{
					// Line of text from GPS Chip
					gps_parse_nmea((char *)msg->data);
				}
			}
			else if (msg->source == BSP_MUX_DLCI_TO_TKVS_SOURCE(VANET_MUXCH_TIMESYNC))
			{
    			if (msg->event == BSP_MUX_EVENT_DATA_RCVD && BSP_TKVS_MSG_HAS_DATA(msg))
    			{
        			if (msg->data[0] == VANET_API_TIME_REQUEST)
                    {
                        vanet_api_timestamp_t t;
                        if (s_gps_state.flags & VAPET_API_GPS_TIME_LOCK)
                        {
                            t.timestamp = bsp_rtc_get_clock();
                            t.timestamp_us = bsp_rtc_get_clock_us() + s_gps_offset_us;
                            if (t.timestamp_us > 1000000)
                            {
                                t.timestamp++;
                                t.timestamp_us -= 1000000;
                            }
                        }
                        else
                        {
                            t.timestamp = t.timestamp_us = 0;
                        }
                        bsp_mux_send(VANET_MUXCH_TIMESYNC, (const uint8_t*) &t, sizeof(t));
                    }                        
    			}
			}
			
			bsp_tkvs_free(msg);
		}
	}
}
