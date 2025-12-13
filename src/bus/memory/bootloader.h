/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_BOOTLOADER_H_
#define CGBL_BOOTLOADER_H_

#include "memory.h"

#define CGBL_BOOTLOADER_DISABLE 0xFF50
#define CGBL_BOOTLOADER_ROM_0_BEGIN 0x0000
#define CGBL_BOOTLOADER_ROM_0_END 0x00FF
#define CGBL_BOOTLOADER_ROM_1_BEGIN 0x0200
#define CGBL_BOOTLOADER_ROM_1_END 0x08FF

#define CGBL_BOOTLOADER_ROM_WIDTH CGBL_WIDTH(CGBL_BOOTLOADER_ROM_1_BEGIN, CGBL_BOOTLOADER_ROM_1_END)

bool cgbl_bootloader_enabled(void);
uint8_t cgbl_bootloader_read(uint16_t address);
void cgbl_bootloader_reset(void);
void cgbl_bootloader_write(uint16_t address, uint8_t data);

#endif /* CGBL_BOOTLOADER_H_ */
