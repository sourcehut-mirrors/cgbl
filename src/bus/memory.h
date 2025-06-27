/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MEMORY_H_
#define CGBL_MEMORY_H_

#include "bus.h"

#define CGBL_MEMORY_RAM_ECHO_0_BEGIN 0xE000
#define CGBL_MEMORY_RAM_ECHO_0_END 0xEFFF
#define CGBL_MEMORY_RAM_ECHO_1_BEGIN 0xF000
#define CGBL_MEMORY_RAM_ECHO_1_END 0xFDFF
#define CGBL_MEMORY_RAM_HIGH_BEGIN 0xFF80
#define CGBL_MEMORY_RAM_HIGH_END 0xFFFE
#define CGBL_MEMORY_RAM_UNUSED_BEGIN 0xFEA0
#define CGBL_MEMORY_RAM_UNUSED_END 0xFEFF
#define CGBL_MEMORY_RAM_WORK_0_BEGIN 0xC000
#define CGBL_MEMORY_RAM_WORK_0_END 0xCFFF
#define CGBL_MEMORY_RAM_WORK_1_BEGIN 0xD000
#define CGBL_MEMORY_RAM_WORK_1_END 0xDFFF
#define CGBL_MEMORY_RAM_WORK_SELECT 0xFF70

#define CGBL_MEMORY_RAM_HIGH_WIDTH \
    CGBL_WIDTH(CGBL_MEMORY_RAM_HIGH_BEGIN, CGBL_MEMORY_RAM_HIGH_END)
#define CGBL_MEMORY_RAM_WORK_WIDTH \
    CGBL_WIDTH(CGBL_MEMORY_RAM_WORK_0_BEGIN, CGBL_MEMORY_RAM_WORK_0_END)

uint8_t cgbl_memory_read(uint16_t address);
cgbl_error_e cgbl_memory_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram);
void cgbl_memory_write(uint16_t address, uint8_t data);

#endif /* CGBL_MEMORY_H_ */
