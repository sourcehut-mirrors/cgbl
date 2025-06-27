/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MAPPER_5_H_
#define CGBL_MAPPER_5_H_

#include "cartridge.h"

#define CGBL_MAPPER_5_RAM_BANK_BEGIN 0x4000
#define CGBL_MAPPER_5_RAM_BANK_END 0x5FFF
#define CGBL_MAPPER_5_RAM_ENABLE_BEGIN 0x0000
#define CGBL_MAPPER_5_RAM_ENABLE_END 0x1FFF
#define CGBL_MAPPER_5_ROM_BANK_HIGH_BEGIN 0x3000
#define CGBL_MAPPER_5_ROM_BANK_HIGH_END 0x3FFF
#define CGBL_MAPPER_5_ROM_BANK_LOW_BEGIN 0x2000
#define CGBL_MAPPER_5_ROM_BANK_LOW_END 0x2FFF

uint8_t cgbl_mapper_5_read(uint16_t address);
void cgbl_mapper_5_reset(void);
void cgbl_mapper_5_write(uint16_t address, uint8_t data);

#endif /* CGBL_MAPPER_5_H_ */
