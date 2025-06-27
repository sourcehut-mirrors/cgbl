/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "mapper_0.h"

uint8_t cgbl_mapper_0_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            result = cgbl_cartridge_ram_read(0, address - CGBL_CARTRIDGE_RAM_BEGIN);
            break;
        case CGBL_CARTRIDGE_ROM_0_BEGIN ... CGBL_CARTRIDGE_ROM_0_END:
            result = cgbl_cartridge_rom_read(0, address - CGBL_CARTRIDGE_ROM_0_BEGIN);
            break;
        case CGBL_CARTRIDGE_ROM_1_BEGIN ... CGBL_CARTRIDGE_ROM_1_END:
            result = cgbl_cartridge_rom_read(1, address - CGBL_CARTRIDGE_ROM_1_BEGIN);
            break;
        default:
            break;
    }
    return result;
}

void cgbl_mapper_0_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_CARTRIDGE_RAM_BEGIN ... CGBL_CARTRIDGE_RAM_END:
            cgbl_cartridge_ram_write(0, address - CGBL_CARTRIDGE_RAM_BEGIN, data);
            break;
        default:
            break;
    }
}
