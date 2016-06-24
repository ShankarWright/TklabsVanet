/**
 *	@file	tkvs.h
 *
 *	@brief	tkLABS Virtual Source
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

#ifndef BSP_TKVS_H
#define BSP_TKVS_H

// requires uC/OS!
#ifdef CONFIG_BSP_UCOS

/**
 * @defgroup group_bsp_services_tkvs TKVS - tkLABS Virtual Source
 *
 * Provide a publish/subscribe mechanism for the platform
 *
 * @{
 */

/// All events
#define BSP_TKVS_ALL_EVENTS 0xffff

/**  
 *  @name Initialization
 *  @{
 */
/// Initialize service
extern void bsp_tkvs_init(void);

/// Initialize TKVS STI Command(s)
extern void bsp_tkvs_sti_init(void);
/// @}

/**
 *  @name Public API
 *  @{
 */

/// TKVS message sources
enum
{
	BSP_TKVS_SRC_CLOCK,                 ///< Clock / Timer. See tkvs_tmr.h
	
#ifdef CONFIG_BSP_ENABLE_TERMIOS
	BSP_TKVS_SRC_TERMIOS_INTERNAL,		///< Termios - see bsp_termios.h
	BSP_TKVS_SRC_TERMIOS_PORT_START,
	BSP_TKVS_SRC_TERMIOS_PORT_END = BSP_TKVS_SRC_TERMIOS_PORT_START + BSP_TERMIOS_COUNT - 1,			
#endif // CONFIG_BSP_ENABLE_TERMIOS

#ifdef CONFIG_BSP_ENABLE_MUX
	BSP_TKVS_SRC_MUX_INTERNAL,			///< Mux - see mux.h
	BSP_TKVS_SRC_MUX_DLCI_START,		
	BSP_TKVS_SRC_MUX_DLCI_END = BSP_TKVS_SRC_MUX_DLCI_START + BSP_TKVS_SRC_MUX_DLCI_NUM - 1,
#endif // CONFIG_BSP_ENABLE_MUX
    
#ifdef CONFIG_BSP_ENABLE_STI
	BSP_TKVS_SRC_STI_INTERNAL,
#endif // CONFIG_BSP_ENABLE_STI

#ifdef CONFIG_BSP_ENABLE_PIN
	BSP_TKVS_SRC_PIN,
#endif // CONFIG_BSP_ENABLE_PIN
	
    BSP_TKVS_SRC_APP_START              ///< Start of application specific sources, see conf_tkvs.h
};

/// TKVS message structure
typedef struct 
{
    struct 
    {
        uint8_t ref;
        uint8_t unused[3];
    } int_hdr;
    
    uint8_t source;                 ///< The publisher
    uint8_t immed_data;             ///< Immediate event data
    uint16_t event;                 ///< The event
	uint16_t data_len;				///< Length of data[] below
    uint8_t data[];                 ///< Variable length event data
} bsp_tkvs_msg_t;

/// Show TKVS Status
extern void bsp_tkvs_status(void);

/// Allocate a message to send
extern bsp_tkvs_msg_t* bsp_tkvs_alloc(uint16_t data_length);

/// Free a message
extern void bsp_tkvs_free(bsp_tkvs_msg_t* msg);

/**
 * Subscribe to a source.  The event_mask can be an OR of several events so that multiple events
 * can be subscribed to at once.
 *
 * When an event is published that matches the event_mask, a const bsp_tkvs_msg_t* is sent to the
 * given OS queue. The received message is read-only and must be freed with bsp_tkvs_free.
 *
 * @param source            The message source
 * @param event_mask        The event(s) to subscribe to
 * @param queue             The OS queue to post the message to
 * @param task_prio         The OS Task Priority
 */
extern bool bsp_tkvs_subscribe(uint8_t source, uint16_t event_mask, OS_EVENT* queue, INT8U task_prio);

/**
 * Publish a message using a pre-allocated message buffer
 *
 * @param source            The message source
 * @param event             The event
 * @param msg               The message, allocated with bsp_tkvs_alloc
 */
extern void bsp_tkvs_publish(uint8_t source, uint16_t event, bsp_tkvs_msg_t* msg);

/**
 * Publish a message with immediate data
 *
 * @param source            The message source
 * @param event             The event
 * @param data              Immediate message data
 */
extern void bsp_tkvs_publish_immed(uint8_t source, uint16_t event, uint8_t data);

/**
 * Publish a message with given data
 *
 * @param source            The message source
 * @param event             The event
 * @param data              The message data
 * @param data_length       The length of the message data
 */
extern void bsp_tkvs_publish_data(uint8_t source, uint16_t event, const void* data, uint16_t data_length);


/**
 * Publish a message with given data and immediate data
 *
 * @param source            The message source
 * @param event             The event
 * @param immed_data		The immediate message data
 * @param data              The message data
 * @param data_length       The length of the message data
 */
extern void bsp_tkvs_publish_immed_with_data(uint8_t source, uint16_t event, uint8_t immed_data, const void* data, uint16_t data_length);

/// Macro to tell is a message has data
#define BSP_TKVS_MSG_HAS_DATA(_msg)	(_msg->data_len != 0)

/**
 * Check if anyone is subscribed to a source/event
 *
 * @param source            The message source
 * @param event             The event
 */
extern bool bsp_tkvs_is_subscribed(uint8_t source, uint16_t event);

/// @}

/// @}

#include "conf_tkvs.h"      // Application defined sources

#endif // CONFIG_BSP_UCOS

#endif // BSP_TKVS_H