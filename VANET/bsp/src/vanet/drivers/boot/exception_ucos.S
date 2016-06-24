/**
 * \file
 *
 * \brief Exception and interrupt vectors mapping for the INTC Software Driver.
 *
 * Copyright (c) 2009-2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#if !__AVR32_UC__ && !__AVR32_AP__
  #error Implementation for the AVR32 architecture only.
#endif

#include <conf_vanet.h>

#ifdef CONFIG_BSP_UCOS

#include <avr32/io.h>

.extern bsp_exception

// uC/OS
.equ    VECTORS_SR_OFFSET,                       0              /* Status  Register offset in System Register               */
.equ    VECTORS_SR_GM_OFFSET,                   16              /* Status  Register, Global Interrupt Mask Offset           */
.equ    VECTORS_SR_MX_OFFSET,                   22              /* Status  Register, Execution Mode Mask offset             */
.equ    VECTORS_SR_MX_EXCEPTION_MODE,   0x00000006              /* Status  Register, Execution Mode Exception               */

.extern OSCtxSw
.extern OSIntISRHandler

//! @{
//! \verbatim


.section  .exception, "ax", @progbits


// Start of Exception Vector Table.

/*
 * EVBA must be aligned with a power of two strictly greater than the
 * EVBA-relative offset of the last vector.
 */
.balign 0x200

// Export symbol.
.global _evba
.type _evba, @function
_evba:

    .org  0x000
    rcall   _cpu_exception
    .org  0x004
    rcall   _cpu_exception
    .org  0x008
    rcall   _cpu_exception
    .org  0x00C
    rcall   _cpu_exception
    .org  0x010
    rcall   _cpu_exception
    .org  0x014
    rcall   _cpu_exception
    .org  0x018
    rcall   _cpu_exception
    .org  0x01C
    rcall   _cpu_exception
    .org  0x020
    rcall   _cpu_exception
    .org  0x024
    rcall   _cpu_exception
    .org  0x028
    rcall   _cpu_exception
    .org  0x02C
    rcall   _cpu_exception
    .org  0x030
    rcall   _cpu_exception
    .org  0x034
    rcall   _cpu_exception
    .org  0x038
    rcall   _cpu_exception
    .org  0x03C
    rcall   _cpu_exception
    .org  0x040
    rcall   _cpu_exception
    .org  0x044
    rcall   _cpu_exception
    .org  0x050
    rcall   _cpu_exception
    .org  0x060
    rcall   _cpu_exception
    .org  0x070
    rcall   _cpu_exception

	.org  0x100
	// Supervisor Call.
    lddpc pc,__OSCtxSw

__OSCtxSw:
    .long   OSCtxSw

_cpu_exception:
    pushm   r11-r12
    mov     r12, lr                             /* lr has the vector number + evba + 2 */
    lda.w   r11, _evba                          /* set r12 = lr - evba - 2 = vector number */
    sub     r12, r11
    sub     r12, 2
    lddsp   r11, sp[12]                         /* set r11 to the PC of the offending instruction */
    lda.w   pc, bsp_exception                   /* jump to the bsp exception handling routine */

/*
 * Interrupt support.
 * The interrupt controller must provide the offset address relative to EVBA.
 * Important note:
 * All interrupts call a C function named _get_interrupt_handler.
 * This function will read group and interrupt line number to then return in
 *R12 a pointer to a user-provided interrupt handler.
 */

.balign 4

.irp    priority, 0, 1, 2, 3
.global _int\priority
.type   _int\priority, @function
_int\priority:
    SSRF    VECTORS_SR_GM_OFFSET                                    /* Set global interrupt mask (disable interrupts)           */
    NOP
    NOP

    MFSR    R12, VECTORS_SR_OFFSET
    BFEXTU  R12, R12, VECTORS_SR_MX_OFFSET, 3
    CP.W    R12, VECTORS_SR_MX_EXCEPTION_MODE                       /* check execution mode                                     */
    BRHS    __exception_stack                                       /* if (R12 == {Exception, NMI}) then change interrupt stack */
                                                                    /* ... to exception stack                                   */

    MOV     R12, \priority                                          /* Pass int_level parameter to BSP_INTC_IntGetHandler       */
    CALL    _get_interrupt_handler
    CP.W    R12, 0                                                  /* Test returned interrupt handler pointer                  */
    BRNE    OSIntISRHandler                                         /* if (R12 != 0) then, jump to the OSIntISRHandler          */
    RETE                                                            /* if (R12 == 0) it was a spurious interrupt then, return   */
                                                                        /* ... from event handler                                   */
.endr

__exception_stack:
    LDDSP   R12, SP[0 * 4]                                          /* Retrieve SR from stack                                   */
    STDSP   SP[6 * 4], R12                                          /* Store SR at bottom of stack                              */
    LDDSP   R12, SP[1 * 4]                                          /* Retrieve PC from stack                                   */
    STDSP   SP[7 * 4], R12                                          /* Store PC at bottom of stack                              */
    LDDSP   R12, SP[3 * 4]                                          /* Retrieve R12 back from stack                             */
    SUB     SP, -6 * 4                                              /* Adjust SP to match exception stack frame                 */
    RETE                                                            /* Return to exception handler                              */

//! \endverbatim
//! @}

#endif // CONFIG_BSP_UCOS