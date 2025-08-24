/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_CARTRIDGE_H_
#define CGBL_CARTRIDGE_H_

#include "memory.h"

#define CGBL_CARTRIDGE_HEADER_CHECKSUM 0x14D
#define CGBL_CARTRIDGE_HEADER_MAPPER 0x147
#define CGBL_CARTRIDGE_HEADER_MODE 0x143
#define CGBL_CARTRIDGE_HEADER_TITLE_BEGIN 0x134
#define CGBL_CARTRIDGE_HEADER_TITLE_END 0x14C
#define CGBL_CARTRIDGE_HEADER_RAM 0x149
#define CGBL_CARTRIDGE_HEADER_ROM 0x148
#define CGBL_CARTRIDGE_RAM_BEGIN 0xA000
#define CGBL_CARTRIDGE_RAM_END 0xBFFF
#define CGBL_CARTRIDGE_RAM_MAGIC 0x004C4247
#define CGBL_CARTRIDGE_ROM_0_BEGIN 0x0000
#define CGBL_CARTRIDGE_ROM_0_END 0x3FFF
#define CGBL_CARTRIDGE_ROM_1_BEGIN 0x4000
#define CGBL_CARTRIDGE_ROM_1_END 0x7FFF

#define CGBL_CARTRIDGE_HEADER_TITLE_WIDTH \
    CGBL_WIDTH(CGBL_CARTRIDGE_HEADER_TITLE_BEGIN, CGBL_CARTRIDGE_HEADER_TITLE_END)
#define CGBL_CARTRIDGE_RAM_WIDTH \
    CGBL_WIDTH(CGBL_CARTRIDGE_RAM_BEGIN, CGBL_CARTRIDGE_RAM_END)
#define CGBL_CARTRIDGE_ROM_WIDTH \
    CGBL_WIDTH(CGBL_CARTRIDGE_ROM_0_BEGIN, CGBL_CARTRIDGE_ROM_0_END)

typedef enum {
    CGBL_CLOCK_SECOND = 1,
    CGBL_CLOCK_MINUTE,
    CGBL_CLOCK_HOUR,
    CGBL_CLOCK_DAY_LOW,
    CGBL_CLOCK_DAY_HIGH,
    CGBL_CLOCK_MAX,
} cgbl_clock_e;

void cgbl_cartridge_clock_latch(void);
uint8_t cgbl_cartridge_clock_read(cgbl_clock_e clock);
void cgbl_cartridge_clock_write(cgbl_clock_e clock, uint8_t data);
uint8_t cgbl_cartridge_palette_hash(char *const disambiguation);
uint16_t cgbl_cartridge_ram_count(void);
uint8_t cgbl_cartridge_ram_read(uint16_t bank, uint16_t address);
void cgbl_cartridge_ram_write(uint16_t bank, uint16_t address, uint8_t data);
uint8_t cgbl_cartridge_read(uint16_t address);
cgbl_error_e cgbl_cartridge_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram);
uint16_t cgbl_cartridge_rom_count(void);
uint8_t cgbl_cartridge_rom_read(uint16_t bank, uint16_t address);
void cgbl_cartridge_step(void);
const char *cgbl_cartridge_title(void);
void cgbl_cartridge_write(uint16_t address, uint8_t data);

#endif /* CGBL_CARTRIDGE_H_ */
