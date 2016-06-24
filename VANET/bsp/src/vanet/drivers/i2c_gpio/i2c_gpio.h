/**
 *	@file	i2c_gpio.h
 *
 *	@brief	I2C GPIO Expanders
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


typedef enum
{
	BSP_I2C_PCA9501,	///< PCA9501 I2C 8-Bit I/O Port
	BSP_I2C_PCF8574,	///< PCF8574 I2C 8-Bit I/O Expander
	BSP_I2C_MCP23017,	///< MCP23017 I2C 16-bit I/O Expander
} bsp_i2c_gpio_type_t;

typedef struct
{
	uint8_t i2c_type;
	uint8_t i2c_addr;
	
	uint16_t data;
	uint16_t ddr;
} bsp_i2c_gpio_private_t;

typedef struct
{
	// Functions GPIO Driver Must Implement
	void (*init)(bsp_i2c_gpio_private_t *p);
	void (*pinMode)(bsp_i2c_gpio_private_t *p, uint8_t pin, uint8_t direction, uint8_t initial_value);
	void (*pullup)(bsp_i2c_gpio_private_t *p, uint8_t pin, bool on_off);
	uint16_t (*readPort)(bsp_i2c_gpio_private_t *p);
	void (*writePort)(bsp_i2c_gpio_private_t *p, uint16_t value);
	uint8_t (*readPin)(bsp_i2c_gpio_private_t *p, uint8_t pin);
	void (*writePin)(bsp_i2c_gpio_private_t *p, uint8_t pin, uint8_t value);
	uint16_t (*readOutputs)(bsp_i2c_gpio_private_t *p);

	// private info
	bsp_i2c_gpio_private_t	p;
} bsp_i2c_gpio_t;

/// Allocate an I2C GPIO Expander
extern bsp_i2c_gpio_t * bsp_i2c_gpio(bsp_i2c_gpio_type_t type, uint8_t i2c_addr);

/// Set the pin direction and initial value if output
static inline void bsp_i2c_gpio_pinMode(bsp_i2c_gpio_t *gpio, uint8_t pin, uint8_t direction, uint8_t initial_value)
{
	gpio->pinMode(&gpio->p, pin, direction, initial_value);
}

/// Write pin output state
static inline void bsp_i2c_gpio_writePin(bsp_i2c_gpio_t *gpio, uint8_t pin, uint8_t value)
{
	gpio->writePin(&gpio->p, pin, value);
}

/// Write all pins output state
static inline void bsp_i2c_gpio_writePort(bsp_i2c_gpio_t *gpio, uint16_t value)
{
	gpio->writePort(&gpio->p, value);
}

/// Read all pins
static inline uint16_t bsp_i2c_gpio_readOutputs(bsp_i2c_gpio_t *gpio)
{
	return gpio->readOutputs(&gpio->p);
}


#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017
void mcp23017_init(bsp_i2c_gpio_private_t* p);
void mcp23017_writePort(bsp_i2c_gpio_private_t *p, uint16_t val);
void mcp23017_writePin(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t val);
void mcp23017_pinMode(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t direction, uint8_t initial_value);
uint16_t mcp23017_readOutputs(bsp_i2c_gpio_private_t *p);
#endif

#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574
void pcf8574_init(bsp_i2c_gpio_private_t* p);
void pcf8574_writePort(bsp_i2c_gpio_private_t *p, uint16_t val);
void pcf8574_writePin(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t val);
void pcf8574_pinMode(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t direction, uint8_t initial_value);
uint16_t pcf8574_readOutputs(bsp_i2c_gpio_private_t *p);
#endif