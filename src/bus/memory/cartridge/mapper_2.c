/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "mapper_2.h"
#include <string.h>

static struct {
    bool enabled;
    uint16_t bank;
} mapper_2 = {};

static void cgbl_mapper_2_update(void) {
    if (!mapper_2.bank) {
        ++mapper_2.bank;
    }
    mapper_2.bank &= cgbl_cartridge_rom_count() - 1;
}

uint8_t cgbl_mapper_2_read(uint16_t address) {
    uint8_t result = 0xFF;
    switch (address) {
    case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
        if (mapper_2.enabled) {
            result = cgbl_cartridge_ram_read(0, (address - CGBL_CARTRIDGE_RAM_BEGIN) & 0x1FF) | 0xF0;
        }
        break;
    case CGBL_CARTRIDGE_ROM_0_BEGIN ... CGBL_CARTRIDGE_ROM_0_END:
        result = cgbl_cartridge_rom_read(0, address - CGBL_CARTRIDGE_ROM_0_BEGIN);
        break;
    case CGBL_CARTRIDGE_ROM_1_BEGIN ... CGBL_CARTRIDGE_ROM_1_END:
        result = cgbl_cartridge_rom_read(mapper_2.bank, address - CGBL_CARTRIDGE_ROM_1_BEGIN);
        break;
    default:
        break;
    }
    return result;
}

void cgbl_mapper_2_reset(void) {
    memset(&mapper_2, 0, sizeof(mapper_2));
    cgbl_mapper_2_update();
}

void cgbl_mapper_2_write(uint16_t address, uint8_t data) {
    switch (address) {
    case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
        if (mapper_2.enabled) {
            cgbl_cartridge_ram_write(0, (address - CGBL_CARTRIDGE_RAM_BEGIN) & 0x1FF, data | 0xF0);
        }
        break;
    case CGBL_MAPPER_2_BANK_SELECT_BEGIN ... CGBL_MAPPER_2_BANK_SELECT_END:
        if (address & 0x100) {
            mapper_2.bank = data & 0xF;
        } else {
            mapper_2.enabled = ((data & 0xF) == 0xA);
        }
        cgbl_mapper_2_update();
        break;
    default:
        break;
    }
}
