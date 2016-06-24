/**
 *	@file	buzzer.c
 *
 *	@brief	Piezo Buzzer Driver
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of CWSI, LLC. Sunrise FL, USA
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_ALERT

#include "conf_tones.h"

// Define the sequences
#define ALERT_SEQ_LINE(N,unused) \
    static const bsp_buzzer_state_t s_alert##N##_seq[] = { BSP_ALERT##N##_SEQ };
MREPEAT(BSP_ALERT_NUM_TONES,ALERT_SEQ_LINE,~)

// Define the tone table
#define ALERT_TONE_LINE(N,unused) \
    { \
        BSP_ALERT##N##_BUZZER, \
        s_alert##N##_seq, \
        sizeof(s_alert##N##_seq) / sizeof(s_alert##N##_seq[0]), \
        BSP_ALERT##N##_REP \
    },

static const struct
{
    int buzzer;
    const bsp_buzzer_state_t* states;
    int state_count;
    int reps;
} s_tone_table[] = {
    MREPEAT(BSP_ALERT_NUM_TONES,ALERT_TONE_LINE,~)
};

#ifdef CONFIG_STI_CMD_ALERT

#define ALERT_TONE_NAME_LINE(N,unused) BSP_ALERT##N##_NAME, 
static const char* s_tone_names[] = {
    MREPEAT(BSP_ALERT_NUM_TONES,ALERT_TONE_NAME_LINE,~)
};

static void bsp_alert_handler(int argc, char** argv, uint8_t port)
{
    if (argc > 1)
    {
        int tone = strtol(argv[1],0,10);
        if (tone < 0)
        {
            bsp_termios_write_str(port, "Stop Alerts\r\n");
            bsp_buzzer_stop();
        }
        else if (tone < BSP_ALERT_NUM_TONES)
        {
            bsp_termios_printf(port, "Start Alert: %s\r\n", s_tone_names[tone]);            
            bsp_alert(tone);
        }
        else
        {
            bsp_termios_write_str(port, "Invalid tone\r\n");
        }
    }
    else
    {
        bsp_termios_write_str(port, "Tone -1: Stop all tones\r\n");
        for (int i=0; i<BSP_ALERT_NUM_TONES; i++)
        {
            bsp_termios_printf(port, "Tone %d: %s\r\n", i, s_tone_names[i]);
        }
    }
}

static bsp_sti_command_t bsp_alert_command =
{
    .name = "alert",
    .handler = &bsp_alert_handler,
    .minArgs = 0,
    .maxArgs = 1,
    STI_HELP("alert                                 Show available tones\r\n"
             "alert <tone>                          Play a tone\r\n"
             "\r\n"
             "Options:\r\n"
             "    <tone>                            Number of the tone to play")

};
#endif // CONFIG_STI_CMD_ALERT

int s_current_alert_tone; // extern this so buzzer can clear it... HACK

void bsp_alert_init(void)
{
    s_current_alert_tone = -1; // we aren't playing a tone right now
    
    #ifdef CONFIG_STI_CMD_ALERT
    bsp_sti_register_command(&bsp_alert_command);
    #endif
}

void bsp_alert(int tone)
{
    if (tone >= 0 && tone < BSP_ALERT_NUM_TONES && tone != s_current_alert_tone)
    {
        bsp_buzzer_start(s_tone_table[tone].buzzer,
                         s_tone_table[tone].states,
                         s_tone_table[tone].state_count,
                         s_tone_table[tone].reps);
        s_current_alert_tone = tone;
    }
}

void bsp_alert_stop(void)
{
    s_current_alert_tone = -1;
    bsp_buzzer_stop();
}

#endif // CONFIG_BSP_ENABLE_BUZZER