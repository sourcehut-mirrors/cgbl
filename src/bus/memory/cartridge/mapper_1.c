/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "mapper_1.h"

static struct {
    struct {
        uint8_t high;
        uint8_t low;
        uint8_t select;
    } bank;
    struct {
        bool enabled;
        uint16_t bank;
    } ram;
    struct {
        uint16_t bank[2];
    } rom;
} mapper_1 = {};

static void cgbl_mapper_1_update(void)
{
    uint16_t count = cgbl_cartridge_rom_count();
    if (count >= 64)
    {
        mapper_1.ram.bank = 0;
        mapper_1.rom.bank[0] = (mapper_1.bank.select & 1) ? (mapper_1.bank.high & 3) << 5 : 0;
        mapper_1.rom.bank[1] = ((mapper_1.bank.high & 3) << 5) | (mapper_1.bank.low & 31);
    }
    else
    {
        mapper_1.ram.bank = (mapper_1.bank.select & 1) ? mapper_1.bank.high & 3 : 0;
        mapper_1.rom.bank[0] = 0;
        mapper_1.rom.bank[1] = mapper_1.bank.low & 31;
    }
    switch (mapper_1.rom.bank[1])
    {
        case 0: case 32: case 64: case 96:
            ++mapper_1.rom.bank[1];
            break;
        default:
            break;
    }
    mapper_1.rom.bank[0] &= count - 1;
    mapper_1.rom.bank[1] &= count - 1;
    count = cgbl_cartridge_ram_count();
    mapper_1.ram.bank &= count - 1;
}

uint8_t cgbl_mapper_1_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            if (mapper_1.ram.enabled)
            {
                result = cgbl_cartridge_ram_read(mapper_1.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN);
            }
            break;
        case CGBL_CARTRIDGE_ROM_0_BEGIN ... CGBL_CARTRIDGE_ROM_0_END:
            result = cgbl_cartridge_rom_read(mapper_1.rom.bank[0], address - CGBL_CARTRIDGE_ROM_0_BEGIN);
            break;
        case CGBL_CARTRIDGE_ROM_1_BEGIN ... CGBL_CARTRIDGE_ROM_1_END:
            result = cgbl_cartridge_rom_read(mapper_1.rom.bank[1], address - CGBL_CARTRIDGE_ROM_1_BEGIN);
            break;
        default:
            break;
    }
    return result;
}

void cgbl_mapper_1_reset(void)
{
    memset(&mapper_1, 0, sizeof (mapper_1));
    cgbl_mapper_1_update();
}

void cgbl_mapper_1_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            if (mapper_1.ram.enabled)
            {
                cgbl_cartridge_ram_write(mapper_1.ram.bank, address - CGBL_CARTRIDGE_RAM_BEGIN, data);
            }
            break;
        case CGBL_MAPPER_1_BANK_HIGH_BEGIN ... CGBL_MAPPER_1_BANK_HIGH_END:
            mapper_1.bank.high = data;
            cgbl_mapper_1_update();
            break;
        case CGBL_MAPPER_1_BANK_LOW_BEGIN ... CGBL_MAPPER_1_BANK_LOW_END:
            mapper_1.bank.low = data;
            cgbl_mapper_1_update();
            break;
        case CGBL_MAPPER_1_BANK_SELECT_BEGIN ... CGBL_MAPPER_1_BANK_SELECT_END:
            mapper_1.bank.select = data;
            cgbl_mapper_1_update();
            break;
        case CGBL_MAPPER_1_RAM_ENABLE_BEGIN ... CGBL_MAPPER_1_RAM_ENABLE_END:
            mapper_1.ram.enabled = ((data & 0xF) == 0xA);
            break;
        default:
            break;
    }
}
