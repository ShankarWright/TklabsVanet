/**
 *	@file	tests.h
 *
 *	@brief	Test routines header
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef TESTS_H
#define TESTS_H

#include <asf.h>

/************************************************************************/
/* Run The Tests!                                                       */
/************************************************************************/
extern void run_tests(void);
extern void show_test_menu(void);

/************************************************************************/
/* Actual list of tests....                                             */
/************************************************************************/
// I2C
void twim_init(void);
void twim_discover_devices(void);
void mpu_6500_basic(void);
void lis3dsh_basic(void);
void i2c_pca9501_test_eeprom(void);
void ds1077_test(void);
void max17043_test(void);
// LCD
void i2c_init_lcd1(void);
void i2c_init_lcd2(void);
void i2c_init_lcd3(void);
void i2c_lcd_test_line_pos(void);
void i2c_lcd_test_moving(void);
void i2c_lcd_basic(void);
void i2c_abstract_class(void);
// MISC
void rtc_enable_tick(void);
void memory_test(void);
void gps_nmea(void);
void piezo_bit_bang(void);
void alix_comm(void);
void mux_test(void);
void pin_gpio_intc_test(void);
void usart_isr_test(void);

#endif // TESTS_H