/*****************************************************************************
 *
 * \file
 *
 * \brief Memory access control configuration file.
 *
 * This file contains the possible external configuration of the memory access
 * control.
 *
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
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
 ******************************************************************************/


 //! Configuration of ctrl_access which is an abstraction layer for memory interfaces (common/services/storage/ctrl_access)

#ifndef _CONF_ACCESS_H_
#define _CONF_ACCESS_H_

#include "compiler.h"
#include "board.h"


#define LUN_0                DISABLE
#define LUN_1                DISABLE
#define LUN_2                DISABLE
#define LUN_3                DISABLE
#define LUN_4                DISABLE
#define LUN_5                DISABLE
#define LUN_6                DISABLE
#define LUN_7                DISABLE
#define LUN_USB              DISABLE

#define LUN_0_INCLUDE                           "sd_mem.h"
#define Lun_0_test_unit_ready                   sd_mem_test_unit_ready
#define Lun_0_read_capacity                     sd_mem_read_capacity
#define Lun_0_wr_protect                        sd_mem_wr_protect
#define Lun_0_removal                           sd_mem_removal
#define Lun_0_mem_2_ram                         sd_mem_mem_2_ram
#define Lun_0_ram_2_mem                         sd_mem_ram_2_mem
#define LUN_0_NAME                              "\"SD/MMC Card over SPI\""

#define ACCESS_USB           false //!< MEM <-> USB interface.
#define ACCESS_MEM_TO_RAM    true  //!< MEM <-> RAM interface.
#define ACCESS_STREAM        true  //!< Streaming MEM <-> MEM interface.
#define ACCESS_STREAM_RECORD false //!< Streaming MEM <-> MEM interface in record mode.
#define ACCESS_MEM_TO_MEM    true //!< MEM <-> MEM interface.
#define ACCESS_CODEC         false //!< Codec interface.
#define GLOBAL_WR_PROTECT    false //!< Management of a global write protection.

#define memory_start_read_action(nb_sectors)
#define memory_stop_read_action()
#define memory_start_write_action(nb_sectors)
#define memory_stop_write_action()

#endif  // _CONF_ACCESS_H_