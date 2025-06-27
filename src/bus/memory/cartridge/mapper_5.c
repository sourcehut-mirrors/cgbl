/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "mapper_5.h"

static struct {
    union {
        struct {
            uint16_t low : 8;
            uint16_t high : 1;
        };
        uint16_t raw;
    } bank;
    struct {
        bool enabled;
        uint16_t bank;
    } ram;
    struct {
        uint16_t bank;
    } rom;
} mapper_5 = {};

static void cgbl_mapper_5_update(void)
{
    mapper_5.rom.bank = mapper_5.bank.raw & 0x1FF;
    mapper_5.rom.bank &= cgbl_cartridge_rom_count() - 1;
    mapper_5.ram.bank &= cgbl_cartridge_ram_count() - 1;
}

uint8_t cgbl_mapper_5_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            if (mapper_5.ram.enabled)
            {
                result = cgbl_cartridge_ram_read(mapper_5.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN);
            }
            break;
        case CGBL_CARTRIDGE_ROM_0_BEGIN ... CGBL_CARTRIDGE_ROM_0_END:
            result = cgbl_cartridge_rom_read(0, address - CGBL_CARTRIDGE_ROM_0_BEGIN);
            break;
        case CGBL_CARTRIDGE_ROM_1_BEGIN ... CGBL_CARTRIDGE_ROM_1_END:
            result = cgbl_cartridge_rom_read(mapper_5.rom.bank, address - CGBL_CARTRIDGE_ROM_1_BEGIN);
            break;
        default:
            break;
    }
    return result;
}

void cgbl_mapper_5_reset(void)
{
    memset(&mapper_5, 0, sizeof (mapper_5));
    cgbl_mapper_5_update();
    mapper_5.rom.bank = 1;
}

void cgbl_mapper_5_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            if (mapper_5.ram.enabled)
            {
                cgbl_cartridge_ram_write(mapper_5.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN, data);
            }
            break;
        case CGBL_MAPPER_5_RAM_BANK_BEGIN ... CGBL_MAPPER_5_RAM_BANK_END:
            mapper_5.ram.bank = data & 0xF;
            cgbl_mapper_5_update();
            break;
        case CGBL_MAPPER_5_RAM_ENABLE_BEGIN ... CGBL_MAPPER_5_RAM_ENABLE_END:
            mapper_5.ram.enabled = ((data & 0xF) == 0xA);
            break;
        case CGBL_MAPPER_5_ROM_BANK_HIGH_BEGIN ... CGBL_MAPPER_5_ROM_BANK_HIGH_END:
            mapper_5.bank.high = data;
            cgbl_mapper_5_update();
            break;
        case CGBL_MAPPER_5_ROM_BANK_LOW_BEGIN ... CGBL_MAPPER_5_ROM_BANK_LOW_END:
            mapper_5.bank.low = data;
            cgbl_mapper_5_update();
            break;
        default:
            break;
    }
}
