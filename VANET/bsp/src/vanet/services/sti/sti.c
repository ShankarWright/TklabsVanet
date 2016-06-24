/**
 *	@file	sti.c
 *
 *	@brief	System Test Interface (STI)
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
#include "vanet.h"
#include <string.h>

#ifdef CONFIG_BSP_ENABLE_STI

extern void bsp_sti_install_builtin(void);

#define PROMPT                      "STI> "
#define IS_WHITESPACE(c)            ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

bsp_sti_command_t *s_commandListHead;

static char s_command[MAX_COMMAND_LENGTH+1];

char s_history[STI_CMD_HISTORY_LEN][MAX_COMMAND_LENGTH+1];
static int s_historyIndex;

static OS_STK s_sti_task_stack[CONFIG_BSP_TASK_STI_STACK_SIZE];
static void bsp_sti_main(void* p_arg);
static OS_EVENT* s_msg_flag;
static void *s_msg_queue[32];
static uint8_t s_termios_sti;

void bsp_sti_init(void)
{
	INT8U perr;
	
	// clear history
	memset(s_history,0,sizeof(s_history));
	
	// initialize vars
	s_commandListHead = 0;
	s_historyIndex = -1;
	
	// debug console message
	print_dbg("Creating STI Task\r\n");
	
	// create a message queue
	s_msg_flag = OSQCreate(&s_msg_queue[0], sizeof(s_msg_queue)/sizeof(s_msg_queue[0]));
	OS_CHECK_NULL(s_msg_flag, "STI task: OSQCreate");
		
	// Create the hw task
	perr = OSTaskCreateExt(bsp_sti_main,
		(void *)0,
		(OS_STK *)&s_sti_task_stack[CONFIG_BSP_TASK_STI_STACK_SIZE - 1],
		CONFIG_BSP_TASK_STI_PRIO,
		CONFIG_BSP_TASK_STI_PRIO,
		(OS_STK *)&s_sti_task_stack[0],
		CONFIG_BSP_TASK_STI_STACK_SIZE,
		(void *)0,
		OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		OS_CHECK_PERR(perr, "STI task: OSTaskCreateExt");
		
	OSTaskNameSet(CONFIG_BSP_TASK_STI_PRIO, (INT8U *)"STI Task", &perr);
	OS_CHECK_PERR(perr, "STI task: OSTaskNameSet");
		
	if (bsp_termios_find_port("Debug", &s_termios_sti))
	{
		// disable buffer so that it is more responsive
		bsp_termios_set_buffer(s_termios_sti, BSP_TERMIOS_BUFFER_NONE);
		bsp_tkvs_subscribe(BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_sti), BSP_TKVS_ALL_EVENTS, s_msg_flag, CONFIG_BSP_TASK_STI_PRIO);
		bsp_tkvs_subscribe(BSP_TKVS_SRC_STI_INTERNAL, BSP_TKVS_ALL_EVENTS, s_msg_flag, CONFIG_BSP_TASK_STI_PRIO);
        
        #ifdef CONFIG_BSP_ENABLE_LOGCAT
        bsp_logcat_set_termios(s_termios_sti);
        #endif
	}
	else
	{
		print_dbg("Unable to find Termios Port - STI will not work!\r\n");
	}
		
	// install internal command
	bsp_sti_install_builtin();
	
}

static void rotate_history(void)
{
	int i;
	if (s_command[0] && strncasecmp(s_history[0],s_command,MAX_COMMAND_LENGTH)) // ignore empty or same command
	{
		for (i=STI_CMD_HISTORY_LEN-1; i>0; i--)
		{
			strncpy(s_history[i],s_history[i-1],MAX_COMMAND_LENGTH);
		}
		strncpy(s_history[0],s_command,MAX_COMMAND_LENGTH);
	}
	s_historyIndex = -1;
}

static void clear_line(void)
{
	bsp_termios_write_str(s_termios_sti, "\r\x1b[K");
	bsp_termios_write_str(s_termios_sti, PROMPT);
	bsp_termios_flush(s_termios_sti);
}

static void load_history(void)
{
	if (s_historyIndex < 0)
	{
		// this clears the line and puts up a new prompt
		clear_line();
		
		// this clears the input buffer
		bsp_termios_set_input(s_termios_sti, "");
	}
	else if (s_historyIndex < STI_CMD_HISTORY_LEN)
	{
		int len = strlen(s_history[s_historyIndex]);
		if (len > 0)
		{
			// this clears the line and puts up a new prompt
			clear_line();
			
			// this loads (and displays) the input buffer with the history
			bsp_termios_set_input(s_termios_sti, s_history[s_historyIndex]);
		}
	}
}

static int parse_command(char** argv)
{
	int argc = 0;
	char* cmd = s_command;
	char* tok = 0;
	char c;
	
	while (true)
	{
		c = *cmd;
		if (!c || argc == STI_MAX_ARGS) break;
		
		if (tok && IS_WHITESPACE(c))
		{
			// we reached the end of a token
			*cmd = 0; // null terminate the token
			argv[argc++] = tok;
			tok = 0;
		}
		else if (!tok && !IS_WHITESPACE(c))
		{
			// we're at the start of a token
			tok = cmd;
		}
		cmd++;
	}
	
	if (tok) argv[argc++] = tok; // don't forget the trailing token
	return argc;
}

static void bsp_sti_process_command(void)
{
	int i;
	char* argv[STI_MAX_ARGS];
	int argc;
	bsp_sti_command_t* nextCmd;
	
	// parse the command
	argc = parse_command(argv);
	
	// exec the command
	for (nextCmd = s_commandListHead; nextCmd; nextCmd = nextCmd->next)
	{
		i = strcasecmp(argv[0],nextCmd->name);
		if (i == 0) // strings are equal
		{
			if (argc > nextCmd->minArgs && argc <= nextCmd->maxArgs+1)
			{
				nextCmd->handler(argc,argv, s_termios_sti);
			}
			else
			{
				bsp_termios_write_str(s_termios_sti, "Invalid args for "); 
				bsp_termios_write_str(s_termios_sti, argv[0]); 
				bsp_termios_write_str(s_termios_sti, "\r\n");
				bsp_termios_flush(s_termios_sti);
				
				if (nextCmd->help)
				{
					bsp_termios_write_str(s_termios_sti, "\r\nUsage:\r\n"); 
					bsp_termios_write_str(s_termios_sti, nextCmd->help); 
					bsp_termios_write_str(s_termios_sti, "\r\n");
					bsp_termios_flush(s_termios_sti);
				}
			}
			break;
		}
		else if (i < 0) // arg name is less than command name
		{
			nextCmd = 0;
			break; // commands are sorted, so not going to find it
		}
	}
	
	if (!nextCmd)
	{
		bsp_termios_write_str(s_termios_sti, "Unknown command: ");
		bsp_termios_write_str(s_termios_sti, argv[0]);
		bsp_termios_write_str(s_termios_sti, "\r\n");
	}
}

void bsp_sti_show_build_info(uint8_t port)
{
	bsp_termios_printf(port, "%s\r\n\r\n", VERSION_ABOUT " (Build " ASTRINGZ(VERSION_SOFTWARE_BUILD) ")");
	bsp_termios_printf(port, "Build Date        : %s\r\n", VERSION_BUILD_DATE);
	bsp_termios_printf(port, "Compiler          : %s\r\n", VERSION_COMPILER);
	bsp_termios_printf(port, "Hardware Revision : %s\r\n", VERSION_HARDWARE_STR);
	bsp_termios_printf(port, "uC/OS OS Version  : %d\r\n", OSVersion());
}

void bsp_sti_main(void* p_arg)
{
	(void) p_arg;
	bsp_tkvs_msg_t *msg;
	INT8U perr;
	
	while (1)
	{
		msg = (bsp_tkvs_msg_t*) OSQPend(s_msg_flag, 0, &perr);
		OS_CHECK_PERR(perr, "STI task: OSQPend");
		if (msg != (void *)0)
		{
			if (msg->source == BSP_TERMIOS_PORT_TO_TKVS_SOURCE(s_termios_sti))
			{
				if (msg->event == BSP_TERMIOS_INPUT_READY)
				{
					// display initial prompt
					bsp_termios_write_str(s_termios_sti, "\r\n");
					bsp_sti_show_build_info(s_termios_sti);
					bsp_termios_write_str(s_termios_sti, "\r\n");
					bsp_termios_write_str(s_termios_sti, PROMPT);
					bsp_termios_flush(s_termios_sti);
				}
				else if (msg->event == BSP_TERMIOS_INPUT_RX)
				{
					if (msg->data_len > 1)		// actual command versus just and empty string
					{
						bsp_termios_write_str(s_termios_sti, "\r\n");
						bsp_termios_flush(s_termios_sti);
						strcpy(s_command, (const char *)msg->data);
						rotate_history();		// requires command in s_command!
						bsp_sti_process_command();
					}
					else
					{
						// empty command
						s_historyIndex = -1;
					}
					bsp_termios_write_str(s_termios_sti, "\r\n");
					bsp_termios_write_str(s_termios_sti, PROMPT);
					bsp_termios_flush(s_termios_sti);
				}
				else if (msg->event == BSP_TERMIOS_INPUT_SIGNAL)
				{
					if (msg->immed_data == BSP_TERMIOS_SIGNAL_INTERRUPT)
					{
						bsp_termios_write_str(s_termios_sti, "^C");
						bsp_termios_write_str(s_termios_sti, "\r\n");
						bsp_termios_write_str(s_termios_sti, PROMPT);
						bsp_termios_flush(s_termios_sti);
					}
					else if (msg->immed_data == BSP_TERMIOS_SIGNAL_UP || msg->immed_data == BSP_TERMIOS_SIGNAL_DOWN)
					{
						int next = s_historyIndex;
						if (msg->immed_data == BSP_TERMIOS_SIGNAL_UP && s_historyIndex < STI_CMD_HISTORY_LEN-1) // up key
						{
							next++;
						}
						else if (msg->immed_data == BSP_TERMIOS_SIGNAL_DOWN && s_historyIndex > -1) // down key
						{
							next--;
						}
						if (next == -1 || (next != s_historyIndex && s_history[next][0]))
						{
							s_historyIndex = next;
							load_history();
						}
					}
				}
			}
			bsp_tkvs_free(msg);
		}
	}
}	
					
void bsp_sti_register_command(bsp_sti_command_t* cmd)
{
	// Insertion sort
	bsp_sti_command_t* nextCmd = s_commandListHead, *prevCmd = 0;
	while (nextCmd)
	{
		if (strcasecmp(cmd->name,nextCmd->name) < 0)
		{
			if (prevCmd)
				prevCmd->next = cmd;
			else
				s_commandListHead = cmd;
				
			cmd->next = nextCmd;
			return;
		}
		
		prevCmd = nextCmd;
		nextCmd = nextCmd->next;
	}
	
	if (prevCmd)
		prevCmd->next = cmd;
	else
		s_commandListHead = cmd;
}

#endif // CONFIG_BSP_ENABLE_STI