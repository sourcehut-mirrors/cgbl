/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MAPPER_3_H_
#define CGBL_MAPPER_3_H_

#include "cartridge.h"

#define CGBL_MAPPER_3_CLOCK_BANK_BEGIN 0x08
#define CGBL_MAPPER_3_CLOCK_BANK_END 0x0C
#define CGBL_MAPPER_3_CLOCK_LATCH_BEGIN 0x6000
#define CGBL_MAPPER_3_CLOCK_LATCH_END 0x7FFF
#define CGBL_MAPPER_3_RAM_BANK_BEGIN 0x4000
#define CGBL_MAPPER_3_RAM_BANK_END 0x5FFF
#define CGBL_MAPPER_3_RAM_ENABLE_BEGIN 0x0000
#define CGBL_MAPPER_3_RAM_ENABLE_END 0x1FFF
#define CGBL_MAPPER_3_ROM_BANK_BEGIN 0x2000
#define CGBL_MAPPER_3_ROM_BANK_END 0x3FFF

uint8_t cgbl_mapper_3_read(uint16_t address);
void cgbl_mapper_3_reset(void);
void cgbl_mapper_3_write(uint16_t address, uint8_t data);

#endif /* CGBL_MAPPER_3_H_ */
