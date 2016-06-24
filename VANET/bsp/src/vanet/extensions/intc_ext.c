#include <asf.h>
#include "vanet.h"

typedef struct {
    bsp_gpio_handler    handler;
    uint32_t            port;
    uint32_t            ifr_mask;
} bsp_gpio_irq_info_t;
    
bsp_gpio_irq_info_t     s_gpio_irq_info[CONFIG_BSP_INTC_MAX_INTERRUPTS];

static uint8_t          s_gpio_irq_cnt = 0;

static void gpio_interrupt_jump_handler(uint8_t port, uint32_t mask)
{
    volatile avr32_gpio_port_t *gpio_port = &AVR32_GPIO.port[port];
    
    for (int i=0; i<s_gpio_irq_cnt; i++)
    {
        
        if (s_gpio_irq_info[i].port == port)
        {
            if (s_gpio_irq_info[i].ifr_mask & mask & gpio_port->ifr)
            {
                // the ifr bit for this handler is set!
                s_gpio_irq_info[i].handler();
                
                // we will only process one handler per interrupt - remove this break to do them all
                break;
            }
        }
    }
};

BSP_INT_ATTR static void gpio_0_handler(void)
{
	gpio_interrupt_jump_handler(0, 0x000000ff);
}
BSP_INT_ATTR static void gpio_1_handler(void)
{
    gpio_interrupt_jump_handler(0, 0x0000ff00);
}
BSP_INT_ATTR static void gpio_2_handler(void)
{
    gpio_interrupt_jump_handler(0, 0x00ff0000);
}
BSP_INT_ATTR static void gpio_3_handler(void)
{
    gpio_interrupt_jump_handler(0, 0xff000000);
}

BSP_INT_ATTR static void gpio_4_handler(void)
{
	gpio_interrupt_jump_handler(1, 0x000000ff);
}
BSP_INT_ATTR static void gpio_5_handler(void)
{
    gpio_interrupt_jump_handler(1, 0x0000ff00);
}
BSP_INT_ATTR static void gpio_6_handler(void)
{
	gpio_interrupt_jump_handler(1, 0x00ff0000);
}
BSP_INT_ATTR static void gpio_7_handler(void)
{
	gpio_interrupt_jump_handler(1, 0xff000000);
}

BSP_INT_ATTR static void gpio_8_handler(void)
{
	gpio_interrupt_jump_handler(2, 0x000000ff);
}
BSP_INT_ATTR static void gpio_9_handler(void)
{
    gpio_interrupt_jump_handler(2, 0x0000ff00);
}
BSP_INT_ATTR static void gpio_10_handler(void)
{
    gpio_interrupt_jump_handler(2, 0x00ff0000);
}
BSP_INT_ATTR static void gpio_11_handler(void)
{
    gpio_interrupt_jump_handler(2, 0xff000000);
}

BSP_INT_ATTR static void gpio_12_handler(void)
{
    gpio_interrupt_jump_handler(3, 0x000000ff);
}
BSP_INT_ATTR static void gpio_13_handler(void)
{
	gpio_interrupt_jump_handler(3, 0x0000ff00);
}
BSP_INT_ATTR static void gpio_14_handler(void)
{
	gpio_interrupt_jump_handler(3, 0x00ff0000);
}
BSP_INT_ATTR static void gpio_15_handler(void)
{
    gpio_interrupt_jump_handler(3, 0xff000000);
}

bool INTC_register_GPIO_interrupt(bsp_gpio_handler handler, uint32_t pin)
{
    uint32_t irq;               // actual INTC IRQ number for the GPIO
    uint32_t ifr_bitmask;       // bitmask of the GPIO in the GPIO[port]->ifr register
    uint8_t port;               // GPIO[port]
    uint8_t offset;             // offset is the INTC signal map block this pin will interrupt on
    
    offset = pin / 8;
    irq = AVR32_GPIO_IRQ_0 + offset;
    port = pin >> 5;
    ifr_bitmask = 1 << (pin & 0x1f);
    
	/*
    print_dbg("Register GPIO: ");
    print_dbg_int(irq); 
    print_dbg(" ");
    print_dbg_int(port);
    print_dbg(" ");
    print_dbg_hex(ifr_bitmask);
    print_dbg("\r\n");
	*/
    
    switch (offset)
    {
	    case 0:
			INTC_register_interrupt(gpio_0_handler, irq, AVR32_INTC_INT0);
			break;
        case 1:
            INTC_register_interrupt(gpio_1_handler, irq, AVR32_INTC_INT0);
            break;
        case 2:
            INTC_register_interrupt(gpio_2_handler, irq, AVR32_INTC_INT0);
            break;
        case 3:
            INTC_register_interrupt(gpio_3_handler, irq, AVR32_INTC_INT0);
            break;
		case 4:
            INTC_register_interrupt(gpio_4_handler, irq, AVR32_INTC_INT0);
            break;
        case 5:
            INTC_register_interrupt(gpio_5_handler, irq, AVR32_INTC_INT0);
            break;
		case 6:
			INTC_register_interrupt(gpio_6_handler, irq, AVR32_INTC_INT0);
			break;
		case 7:
			INTC_register_interrupt(gpio_7_handler, irq, AVR32_INTC_INT0);
			break;
		case 8:
			INTC_register_interrupt(gpio_8_handler, irq, AVR32_INTC_INT0);
			break;								
        case 9:
            INTC_register_interrupt(gpio_9_handler, irq, AVR32_INTC_INT0);
            break;
        case 10:
            INTC_register_interrupt(gpio_10_handler, irq, AVR32_INTC_INT0);
            break;
        case 11:
            INTC_register_interrupt(gpio_11_handler, irq, AVR32_INTC_INT0);
            break;
        case 12:
            INTC_register_interrupt(gpio_12_handler, irq, AVR32_INTC_INT0);
            break;
        case 13:
            INTC_register_interrupt(gpio_13_handler, irq, AVR32_INTC_INT0);
            break;
        case 14:
            INTC_register_interrupt(gpio_14_handler, irq, AVR32_INTC_INT0);
            break;
        case 15:
            INTC_register_interrupt(gpio_15_handler, irq, AVR32_INTC_INT0);
            break;
        default:
            print_dbg("Unsupported GPIO Signal Map ");
            print_dbg_int(offset);
            print_dbg("\r\n");
            break;
    }
    
    s_gpio_irq_info[s_gpio_irq_cnt].handler = handler;
    s_gpio_irq_info[s_gpio_irq_cnt].ifr_mask = ifr_bitmask;
    s_gpio_irq_info[s_gpio_irq_cnt].port = port;
    s_gpio_irq_cnt++;
    
    return true;
}