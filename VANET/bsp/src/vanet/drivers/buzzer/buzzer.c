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

#ifdef CONFIG_BSP_ENABLE_BUZZER

typedef struct 
{
    const char* name;
    bool externally_driven;
    union
    {
        struct // used for externally driven
        {
            volatile avr32_tc_t* timer;     
            unsigned int channel;
        } ex;
        
        struct // used for internally driven
        {
            unsigned int pin1;
            unsigned int pin2;
            int active_level;
        } in;
    };
} channel_def_t;

static const channel_def_t s_buzzers[] = {
    #define BUZZER_CHANNEL_INFO_LINE(N,unused) CONFIG_BSP_BUZZER##N,
    MREPEAT(CONFIG_BSP_BUZZER_COUNT,BUZZER_CHANNEL_INFO_LINE,~)
};

const channel_def_t* s_buzzer;
const bsp_buzzer_state_t* s_states;
uint32_t s_state_count;
int32_t s_current_state;
int32_t s_repetitions;
uint32_t s_period_counter;
extern int s_current_alert_tone;

#ifdef CONFIG_STI_CMD_BUZZ
static void bsp_buzz_handler(int argc, char** argv, uint8_t port)
{
    if (argc > 1)
    {
        int channel = strtoul(argv[1],0,10);
        if (channel >= 0 && channel < CONFIG_BSP_BUZZER_COUNT)
        {
            if (argc > 2)
            {
                uint16_t hz = strtoul(argv[2],0,10);
                uint32_t on_time = 0;
                uint32_t off_time = 0;
                uint32_t reps = 1;
                
                if (argc > 3) on_time = strtoul(argv[3],0,10);
                if (argc > 4) off_time = strtoul(argv[4],0,10), reps = -1;
                if (argc > 5) reps = strtoul(argv[5],0,10);
                
                bsp_termios_printf(port, "Buzz (%d,%d,%d,%d)\r\n",hz,on_time,off_time,reps);
                
                bsp_buzz_periodic(channel,hz,on_time,off_time,reps);
            }
            else
            {
                bsp_buzzer_stop();
                bsp_termios_write_str(port, "Buzzer stopped\r\n");
            }
        }
        else
        {
            bsp_termios_write_str(port, "Invalid channel\r\n");
        }
    }
    else
    {
        for (int i=0; i<CONFIG_BSP_BUZZER_COUNT; i++)
        {
            bsp_termios_printf(port, "Channel %d: %s - ", i, s_buzzers[i].name);
            if (s_buzzers[i].externally_driven) bsp_termios_write_str(port, "Externally Driven");
            else  bsp_termios_write_str(port,  "Internally Driven");
            bsp_termios_write_str(port, "\r\n");
        }
    }
}

static bsp_sti_command_t bsp_buzz_command =
{
    .name = "buzz",
    .handler = &bsp_buzz_handler,
    .minArgs = 0,
    .maxArgs = 5,
    STI_HELP("buzz                                  List buzzers\r\n"
             "buzz <ch>                             Stop a buzzer\r\n"
             "buzz <ch> <hz>                        Play continuous tone\r\n"
             "buzz <ch> <hz> <on>                   Play tone for <on> ms\r\n"
             "buzz <ch> <hz> <on> <off>             Repeat tone with on/off time\r\n"
             "buzz <ch> <hz> <on> <off> <N>         Repeat on/off tone <N> times\r\n"
             "\r\n"
             "Options:\r\n"
             "    <ch>                              Which buzzer channel\r\n"
             "    <hz>                              Buzzer frequency in Hz\r\n"
             "    <on>                              Active period in ms\r\n"
             "    <off>                             Inactive period in ms\r\n"
             "    <N>                               Number of repetitions")
};
#endif // CONFIG_STI_CMD_BUZZ

static void enable_output(const channel_def_t* channel, int freq)
{
    if (channel->externally_driven)
    {
        uint32_t clock = sysclk_get_peripheral_bus_hz(channel->ex.timer) / 32; // clock source is PB / 32
        uint32_t rc = clock / freq;
            
        // check for over/underflow
        if (rc > 0xffff) rc = 0xffff;
        else if (rc < 8) rc = 8;

        // set up RA, RB, and RC.  always use 50% duty cycle (RA = RB = RC/2)
        tc_write_ra(channel->ex.timer, channel->ex.channel, (uint16_t)rc / 2);
        tc_write_rb(channel->ex.timer, channel->ex.channel, (uint16_t)rc / 2);
        tc_write_rc(channel->ex.timer, channel->ex.channel, (uint16_t)rc);
        tc_start(channel->ex.timer, channel->ex.channel);
    }
    else if (channel->in.active_level)
    {
        gpio_set_pin_high(channel->in.pin1);
    }
    else
    {
        gpio_set_pin_low(channel->in.pin1);
    }
}

static void disable_output(const channel_def_t* channel)
{
    if (channel->externally_driven)
    {
        // software trigger to clear the output, turn off the timer
        tc_software_trigger(channel->ex.timer, channel->ex.channel);
        tc_stop(channel->ex.timer, channel->ex.channel);
    }
    else if (channel->in.active_level)
    {
        gpio_set_pin_low(channel->in.pin1);
    }
    else
    {
        gpio_set_pin_high(channel->in.pin1);
    }
}

static void reset_timeout(void)
{
    if (s_period_counter > 0)
    {
        // see how long to wait
        uint16_t rc = min(0xffff,s_period_counter);
        tc_write_rc(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL, rc);
        
        // remember how much we've waited
        s_period_counter -= rc;

        // Reset the counter
        tc_start(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL);
    }
    else
    {
        // make sure the timer is stopped
        tc_stop(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL);
    }
}

static void set_timeout(uint16_t ms)
{
    // clock source is OSC32K
    s_period_counter = BOARD_OSC32_HZ * ms / 1000;
    
    // Reset the counter
    reset_timeout();
}

static void next_state(void)
{
    s_current_state++;
    if (s_current_state == s_state_count)
    {
        if (s_repetitions > 0)
        {
            s_repetitions--;
        }

        // we've reached the end of the sequence, repeat
        if (s_repetitions == 0)
        {
            // no more repetitions, stop
            bsp_buzzer_stop();
            return;
        }
            
        // go back to the first state
        s_current_state = 0;
    }
    
    const bsp_buzzer_state_t* state = &s_states[s_current_state];
    if (state->freq > 0)
    {
        enable_output(s_buzzer,state->freq);
    }
    else
    {
        disable_output(s_buzzer);
    }
    set_timeout(state->ms);        
}

static void tc_irq(void)
{
    // clear the interrupt flag by reading the status register
    tc_read_sr(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL);
    
    // run the state machine
    if (s_buzzer)
    {
        if (s_period_counter == 0)
        {
            next_state();
        }
        else
        {
            reset_timeout();
        }
    }
}

static void init_waveform_timers(void)
{
    // Common waveform generation options
    tc_waveform_opt_t waveform_opt =
    {
        .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,     // Count up, reset when counter hits RC
        .tcclks   = TC_CLOCK_SOURCE_TC4,                    // Internal source clock 4 = PB / 32
        
        .aswtrg   = TC_EVT_EFFECT_CLEAR,                    // TIOA: set low on software trigger
        .aeevt    = TC_EVT_EFFECT_NOOP,                     // TIOA: no external trigger
        .acpc     = TC_EVT_EFFECT_CLEAR,                    // TIOA: set low on RC compare
        .acpa     = TC_EVT_EFFECT_SET,                      // TIOA: set high on RA compare
        
        .bswtrg   = TC_EVT_EFFECT_CLEAR,                    // TIOB: set low on software trigger
        .beevt    = TC_EVT_EFFECT_NOOP,                     // TIOB: no external trigger
        .bcpc     = TC_EVT_EFFECT_SET,                      // TIOB: set high on RC compare
        .bcpb     = TC_EVT_EFFECT_CLEAR,                    // TIOB: set low on RB compare

        .enetrg   = false,                                  // No external trigger
        .eevt     = TC_EXT_EVENT_SEL_XC0_OUTPUT,            // No external trigger
        .eevtedg  = TC_SEL_NO_EDGE,                         // No external trigger
        .cpcdis   = false,                                  // Do not disable counter when it hits RC
        .cpcstop  = false,                                  // Do not stop counter when it hits RC
        .burst    = TC_BURST_NOT_GATED,                     // No burst clock
        .clki     = TC_CLOCK_RISING_EDGE,                   // No clock inversion
    };
    
    // Initialize the buzzer waveform timers
    for (int i=0; i<CONFIG_BSP_BUZZER_COUNT; i++)
    {
        if (s_buzzers[i].externally_driven)
        {
            // Enable the clock to the timer
            sysclk_enable_peripheral_clock(s_buzzers[i].ex.timer);
        
            // Set up the timer in waveform mode
            waveform_opt.channel = s_buzzers[i].ex.channel;
            tc_init_waveform(s_buzzers[i].ex.timer, &waveform_opt);        
        }
        else
        {
            uint32_t flags = s_buzzers[i].in.active_level ? GPIO_INIT_LOW : GPIO_INIT_HIGH;
            flags |= GPIO_DIR_OUTPUT;
            
            gpio_configure_pin(s_buzzers[i].in.pin1, flags);
            if (s_buzzers[i].in.pin2 < 1000) gpio_configure_pin(s_buzzers[i].in.pin2, flags);
        }
    }
}    

static void init_repetition_timer(void)
{
    // Waveform generation options
    tc_waveform_opt_t waveform_opt =
    {
        .channel  = CONFIG_BSP_BUZZER_REP_CHANNEL,
        .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,     // Count up, reset when counter hits RC
        .tcclks   = TC_CLOCK_SOURCE_TC1,                    // Internal source clock 1 = OSC32K
        .aswtrg   = TC_EVT_EFFECT_NOOP,
        .aeevt    = TC_EVT_EFFECT_NOOP,
        .acpc     = TC_EVT_EFFECT_NOOP,
        .acpa     = TC_EVT_EFFECT_NOOP,
        .bswtrg   = TC_EVT_EFFECT_NOOP,
        .beevt    = TC_EVT_EFFECT_NOOP,
        .bcpc     = TC_EVT_EFFECT_NOOP,
        .bcpb     = TC_EVT_EFFECT_NOOP,
        .enetrg   = false,                                  // No external trigger
        .eevt     = TC_EXT_EVENT_SEL_XC0_OUTPUT,            // No external trigger
        .eevtedg  = TC_SEL_NO_EDGE,                         // No external trigger
        .cpcdis   = false,                                  // Do not disable counter when it hits RC
        .cpcstop  = true,                                   // Stop counter when it hits RC
        .burst    = TC_BURST_NOT_GATED,                     // No burst clock
        .clki     = TC_CLOCK_RISING_EDGE,                   // No clock inversion
    };
    
    // Timer interrupt options
    tc_interrupt_t tc_interrupt = { 0 };
    tc_interrupt.cpcs = 1; // interrupt on RC compare

    // Enable the clock to the timer
    sysclk_enable_peripheral_clock(CONFIG_BSP_BUZZER_REP_TIMER);

    // Set up the timer in waveform mode
    tc_init_waveform(CONFIG_BSP_BUZZER_REP_TIMER, &waveform_opt);
    
    // Configure the interrupt
    tc_configure_interrupts(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL, &tc_interrupt);
    INTC_register_interrupt(&tc_irq, CONFIG_BSP_BUZZER_REP_IRQ, CONFIG_BSP_BUZZER_REP_IRQ_PRI);    
}    

void bsp_buzzer_init(void)
{
    // Set up the GPIOs
    #define BUZZER_GPIO_LINE(N,unused) CONFIG_BSP_BUZZER##N##_GPIO_MAP
    const gpio_map_t gpio_map = {
        MREPEAT(CONFIG_BSP_BUZZER_COUNT,BUZZER_GPIO_LINE,~)
    };
    gpio_enable_module(gpio_map, sizeof(gpio_map) / sizeof(gpio_map[0]));
    
    // Initialize the timers
    init_waveform_timers();
    init_repetition_timer();

    #ifdef CONFIG_STI_CMD_BUZZ
    // install our PTI command
    bsp_sti_register_command(&bsp_buzz_command);
    #endif
}

void bsp_buzz_periodic(int buzzer, uint16_t hz, uint16_t on_time, uint16_t off_time, int reps)
{
    static bsp_buzzer_state_t states[CONFIG_BSP_BUZZER_COUNT][2];
    
    bsp_buzzer_stop();
    
    states[buzzer][0].freq = hz;
    states[buzzer][0].ms = on_time;
    if (off_time > 0)
    {
        states[buzzer][1].freq = 0;
        states[buzzer][1].ms = off_time;
        bsp_buzzer_start(buzzer, states[buzzer], 2, reps);
    }
    else
    {
        bsp_buzzer_start(buzzer, states[buzzer], 1, reps);
    }
}

void bsp_buzzer_start(int buzzer, const bsp_buzzer_state_t* states, int state_count, int reps)
{
    if (buzzer < 0 || buzzer >= CONFIG_BSP_BUZZER_COUNT) return;
    
    // in case we're already running, stop
    bsp_buzzer_stop();

    // store the states
    s_buzzer = &s_buzzers[buzzer];

    s_states = states;
    s_state_count = state_count;
    s_repetitions = reps;
    s_current_state = -1;        
    
    // don't let us turn off the PB clock
    sleepmgr_vote_preferred_sleep(SYS_BUZZER, SLEEPMGR_FROZEN);
    
    // and we're off
    next_state();
}

void bsp_buzzer_stop(void)
{
    if (s_buzzer)
    {
        // make sure the repetition timer is stopped
        tc_stop(CONFIG_BSP_BUZZER_REP_TIMER, CONFIG_BSP_BUZZER_REP_CHANNEL);

        // make sure the output is off
        disable_output(s_buzzer);
    
        // clear this in case some interrupt is pending or something
        s_buzzer = 0;
        s_current_alert_tone = -1;
    
        // clear our sleep vote
        sleepmgr_abstain_preferred_sleep(SYS_BUZZER);
    }
}

#endif // CONFIG_BSP_ENABLE_BUZZER