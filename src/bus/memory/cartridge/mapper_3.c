/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "mapper_3.h"
#include <string.h>

static struct {
    struct {
        bool latched;
        cgbl_clock_e bank;
    } clock;
    struct {
        bool enabled;
        uint16_t bank;
    } ram;
    struct {
        uint16_t bank;
    } rom;
} mapper_3 = {};

static void cgbl_mapper_3_update(void) {
    if (!mapper_3.rom.bank) {
        ++mapper_3.rom.bank;
    }
    mapper_3.rom.bank &= cgbl_cartridge_rom_count() - 1;
    mapper_3.ram.bank &= cgbl_cartridge_ram_count() - 1;
}

uint8_t cgbl_mapper_3_read(uint16_t address) {
    uint8_t result = 0xFF;
    switch (address) {
    case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
        if (mapper_3.ram.enabled) {
            switch (mapper_3.clock.bank) {
            case CGBL_CLOCK_SECOND ... CGBL_CLOCK_DAY_HIGH:
                result = cgbl_cartridge_clock_read(mapper_3.clock.bank);
                break;
            default:
                result = cgbl_cartridge_ram_read(mapper_3.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN);
                break;
            }
        }
        break;
    case CGBL_CARTRIDGE_ROM_0_BEGIN ... CGBL_CARTRIDGE_ROM_0_END:
        result = cgbl_cartridge_rom_read(0, address - CGBL_CARTRIDGE_ROM_0_BEGIN);
        break;
    case CGBL_CARTRIDGE_ROM_1_BEGIN ... CGBL_CARTRIDGE_ROM_1_END:
        result = cgbl_cartridge_rom_read(mapper_3.rom.bank, address - CGBL_CARTRIDGE_ROM_1_BEGIN);
        break;
    default:
        break;
    }
    return result;
}

void cgbl_mapper_3_reset(void) {
    memset(&mapper_3, 0, sizeof(mapper_3));
    cgbl_mapper_3_update();
}

void cgbl_mapper_3_write(uint16_t address, uint8_t data) {
    switch (address) {
    case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
        if (mapper_3.ram.enabled) {
            switch (mapper_3.clock.bank) {
            case CGBL_CLOCK_SECOND ... CGBL_CLOCK_DAY_HIGH:
                cgbl_cartridge_clock_write(mapper_3.clock.bank, data);
                break;
            default:
                cgbl_cartridge_ram_write(mapper_3.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN, data);
                break;
            }
        }
        break;
    case CGBL_MAPPER_3_CLOCK_LATCH_BEGIN ... CGBL_MAPPER_3_CLOCK_LATCH_END:
        if (!data && !mapper_3.clock.latched) {
            mapper_3.clock.latched = true;
        } else if (data && mapper_3.clock.latched) {
            mapper_3.clock.latched = false;
            cgbl_cartridge_clock_latch();
        }
        break;
    case CGBL_MAPPER_3_RAM_BANK_BEGIN ... CGBL_MAPPER_3_RAM_BANK_END:
        switch (data) {
        case CGBL_MAPPER_3_CLOCK_BANK_BEGIN ... CGBL_MAPPER_3_CLOCK_BANK_END:
            mapper_3.clock.bank = (data - CGBL_MAPPER_3_CLOCK_BANK_BEGIN) + CGBL_CLOCK_SECOND;
            break;
        default:
            mapper_3.clock.bank = 0;
            mapper_3.ram.bank = data & 3;
            cgbl_mapper_3_update();
            break;
        }
        break;
    case CGBL_MAPPER_3_RAM_ENABLE_BEGIN ... CGBL_MAPPER_3_RAM_ENABLE_END:
        mapper_3.ram.enabled = ((data & 0xF) == 0xA);
        break;
    case CGBL_MAPPER_3_ROM_BANK_BEGIN ... CGBL_MAPPER_3_ROM_BANK_END:
        mapper_3.rom.bank = data & 0x7F;
        cgbl_mapper_3_update();
        break;
    default:
        break;
    }
}
