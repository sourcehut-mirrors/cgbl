/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mapper_0.h"
#include "mapper_1.h"
#include "mapper_2.h"
#include "mapper_3.h"
#include "mapper_5.h"

typedef struct __attribute__((packed)) {
    union {
        uint8_t counter : 6;
        uint8_t raw;
    } second;
    union {
        uint8_t counter : 6;
        uint8_t raw;
    } minute;
    union {
        uint8_t counter : 5;
        uint8_t raw;
    } hour;
    union {
        struct {
            uint16_t counter : 9;
            uint16_t : 5;
            uint16_t halt : 1;
            uint16_t carry : 1;
        };
        struct {
            uint8_t low;
            uint8_t high;
        };
    } day;
} cgbl_clock_t;

typedef struct {
    uint8_t (*read)(uint16_t address);
    void (*reset)(void);
    void (*write)(uint16_t address, uint8_t data);
} cgbl_mapper_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t length;
    struct {
        struct {
            uint8_t major;
            uint8_t minor;
        } version;
        uint8_t reserved;
    } attribute;
    cgbl_clock_t clock;
    uint8_t data[];
} cgbl_ram_t;

static const struct {
    uint8_t type;
    cgbl_mapper_t mapper;
} MAPPER[] = {
    { -1, { NULL, NULL, NULL, }, },
    { 0, { cgbl_mapper_0_read, NULL, cgbl_mapper_0_write, }, },
    { 1, { cgbl_mapper_1_read, cgbl_mapper_1_reset, cgbl_mapper_1_write, }, },
    { 2, { cgbl_mapper_1_read, cgbl_mapper_1_reset, cgbl_mapper_1_write, }, },
    { 3, { cgbl_mapper_1_read, cgbl_mapper_1_reset, cgbl_mapper_1_write, }, },
    { 5, { cgbl_mapper_2_read, cgbl_mapper_2_reset, cgbl_mapper_2_write, }, },
    { 6, { cgbl_mapper_2_read, cgbl_mapper_2_reset, cgbl_mapper_2_write, }, },
    { 8, { cgbl_mapper_0_read, NULL, cgbl_mapper_0_write, }, },
    { 9, { cgbl_mapper_0_read, NULL, cgbl_mapper_0_write, }, },
    { 15, { cgbl_mapper_3_read, cgbl_mapper_3_reset, cgbl_mapper_3_write, }, },
    { 16, { cgbl_mapper_3_read, cgbl_mapper_3_reset, cgbl_mapper_3_write, }, },
    { 17, { cgbl_mapper_3_read, cgbl_mapper_3_reset, cgbl_mapper_3_write, }, },
    { 18, { cgbl_mapper_3_read, cgbl_mapper_3_reset, cgbl_mapper_3_write, }, },
    { 19, { cgbl_mapper_3_read, cgbl_mapper_3_reset, cgbl_mapper_3_write, }, },
    { 25, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
    { 26, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
    { 27, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
    { 28, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
    { 29, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
    { 30, { cgbl_mapper_5_read, cgbl_mapper_5_reset, cgbl_mapper_5_write, }, },
};

static const uint16_t RAM[] = {
    1, 1, 1, 4, 16, 8,
};

static const uint16_t ROM[] = {
    2, 4, 8, 16, 32, 64, 128, 256, 512,
};

static struct {
    uint8_t hash;
    char title[CGBL_CARTRIDGE_HEADER_TITLE_WIDTH];
    const cgbl_mapper_t *mapper;
    struct {
        uint32_t delay;
        cgbl_clock_t latch;
    } clock;
    struct {
        uint16_t count;
        uint8_t *data;
        cgbl_clock_t *clock;
    } ram;
    struct {
        uint16_t count;
        const uint8_t *data;
    } rom;
} cartridge = {};

static void cgbl_cartridge_hash_reset(void)
{
    const char *title = (const char *)&cartridge.rom.data[CGBL_CARTRIDGE_HEADER_TITLE_BEGIN];
    for (uint8_t index = 0; index < 16; ++index)
    {
        cartridge.hash += title[index];
    }
}

static cgbl_error_e cgbl_cartridge_mapper_reset(void)
{
    uint8_t type = cartridge.rom.data[CGBL_CARTRIDGE_HEADER_MAPPER];
    for (uint8_t index = 1; index < CGBL_LENGTH(MAPPER); ++index)
    {
        if (type == MAPPER[index].type)
        {
            cartridge.mapper = &MAPPER[index].mapper;
            if (cartridge.mapper->reset)
            {
                cartridge.mapper->reset();
            }
            break;
        }
    }
    if (!cartridge.mapper)
    {
        return CGBL_ERROR("Unsupported mapper: %02X", type);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_cartridge_ram_reset(cgbl_bank_t *const bank)
{
    uint8_t count = 0;
    uint32_t length = 0;
    cgbl_error_e result = CGBL_SUCCESS;
    cgbl_ram_t *ram = (cgbl_ram_t *)bank->data;
    if (bank->length < sizeof (*ram))
    {
        return CGBL_ERROR("Invalid ram length: %u bytes", bank->length);
    }
    if ((count = cartridge.rom.data[CGBL_CARTRIDGE_HEADER_RAM]) >= CGBL_LENGTH(RAM))
    {
        return CGBL_ERROR("Unsupported ram type: %02X", count);
    }
    cartridge.ram.count = RAM[count];
    length = (cartridge.ram.count * CGBL_CARTRIDGE_RAM_WIDTH) + sizeof (*ram);
    if (bank->length < length)
    {
        return CGBL_ERROR("Invalid ram length: %u bytes", bank->length);
    }
    if (ram->magic == CGBL_CARTRIDGE_RAM_MAGIC)
    {
        if ((ram->attribute.version.major > CGBL_VERSION_MAJOR)
            || (ram->attribute.version.major > CGBL_VERSION_MINOR)
            || (ram->attribute.reserved != 0))
        {
            return CGBL_ERROR("Unsupported ram header attributes");
        }
        if (ram->length != (cartridge.ram.count * CGBL_CARTRIDGE_RAM_WIDTH))
        {
            return CGBL_ERROR("Invalid ram header length: %u bytes", ram->length);
        }
    }
    else
    {
        memset(ram, 0, sizeof (*ram));
        ram->magic = CGBL_CARTRIDGE_RAM_MAGIC;
        ram->attribute.version.major = CGBL_VERSION_MAJOR;
        ram->attribute.version.minor = CGBL_VERSION_MINOR;
        ram->length = length - sizeof (*ram);
    }
    bank->length = length;
    cartridge.ram.clock = &ram->clock;
    cartridge.ram.data = ram->data;
    return result;
}

static cgbl_error_e cgbl_cartridge_rom_reset(const cgbl_bank_t *const bank)
{
    uint8_t checksum = 0, count = 0;
    if (bank->length < CGBL_CARTRIDGE_ROM_WIDTH)
    {
        return CGBL_ERROR("Invalid rom length: %u bytes", bank->length);
    }
    for (uint16_t address = CGBL_CARTRIDGE_HEADER_TITLE_BEGIN; address <= CGBL_CARTRIDGE_HEADER_TITLE_END; ++address)
    {
        checksum = checksum - bank->data[address] - 1;
    }
    if (checksum != bank->data[CGBL_CARTRIDGE_HEADER_CHECKSUM])
    {
        return CGBL_ERROR("Invalid rom checksum: %02X", checksum);
    }
    if ((count = bank->data[CGBL_CARTRIDGE_HEADER_ROM]) >= CGBL_LENGTH(ROM))
    {
        return CGBL_ERROR("Unsupported rom type: %02X", count);
    }
    cartridge.rom.count = ROM[count];
    if (bank->length != (cartridge.rom.count * CGBL_CARTRIDGE_ROM_WIDTH))
    {
        return CGBL_ERROR("Invalid rom length: %u bytes", bank->length);
    }
    cartridge.rom.data = bank->data;
    return CGBL_SUCCESS;
}

static void cgbl_cartridge_title_reset(void)
{
    const char *title = (const char *)&cartridge.rom.data[CGBL_CARTRIDGE_HEADER_TITLE_BEGIN];
    for (uint8_t index = 0; index < CGBL_LENGTH(cartridge.title); ++index)
    {
        cartridge.title[index] = title[index];
        if (cartridge.title[index] && (!isprint(cartridge.title[index]) || isspace(cartridge.title[index])))
        {
            cartridge.title[index] = ' ';
        }
    }
    if (!strlen(cartridge.title))
    {
        snprintf(cartridge.title, CGBL_LENGTH(cartridge.title), "UNTITLED");
    }
}

void cgbl_cartridge_clock_latch(void)
{
    memcpy(&cartridge.clock.latch, cartridge.ram.clock, sizeof (*cartridge.ram.clock));
}

uint8_t cgbl_cartridge_clock_read(cgbl_clock_e clock)
{
    uint8_t result = 0xFF;
    switch (clock)
    {
        case CGBL_CLOCK_DAY_HIGH:
            result = cartridge.clock.latch.day.high;
            break;
        case CGBL_CLOCK_DAY_LOW:
            result = cartridge.clock.latch.day.low;
            break;
        case CGBL_CLOCK_HOUR:
            result = cartridge.clock.latch.hour.raw;
            break;
        case CGBL_CLOCK_MINUTE:
            result = cartridge.clock.latch.minute.raw;
            break;
        case CGBL_CLOCK_SECOND:
            result = cartridge.clock.latch.second.raw;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_cartridge_clock_write(cgbl_clock_e clock, uint8_t data)
{
    switch (clock)
    {
        case CGBL_CLOCK_DAY_HIGH:
            cartridge.ram.clock->day.high = data & 0xC1;
            break;
        case CGBL_CLOCK_DAY_LOW:
            cartridge.ram.clock->day.low = data;
            break;
        case CGBL_CLOCK_HOUR:
            cartridge.ram.clock->hour.counter = data;
            break;
        case CGBL_CLOCK_MINUTE:
            cartridge.ram.clock->minute.counter = data;
            break;
        case CGBL_CLOCK_SECOND:
            cartridge.ram.clock->second.counter = data;
            break;
        default:
            break;
    }
}

uint8_t cgbl_cartridge_palette_hash(char *const disambiguation)
{
    *disambiguation = cartridge.title[3];
    return cartridge.hash;
}

uint16_t cgbl_cartridge_ram_count(void)
{
    return cartridge.ram.count;
}

uint8_t cgbl_cartridge_ram_read(uint16_t bank, uint16_t address)
{
    return cartridge.ram.data[(bank * CGBL_CARTRIDGE_RAM_WIDTH) + address];
}

void cgbl_cartridge_ram_write(uint16_t bank, uint16_t address, uint8_t data)
{
    cartridge.ram.data[(bank * CGBL_CARTRIDGE_RAM_WIDTH) + address] = data;
}

uint8_t cgbl_cartridge_read(uint16_t address)
{
    uint8_t result = 0xFF;
    if (cartridge.mapper->read)
    {
        result = cartridge.mapper->read(address);
    }
    return result;
}

cgbl_error_e cgbl_cartridge_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&cartridge, 0, sizeof (cartridge));
    cartridge.clock.delay = 4213440;
    if (ram->data && rom->data)
    {
        if (((result = cgbl_cartridge_rom_reset(rom)) == CGBL_SUCCESS)
            && ((result = cgbl_cartridge_ram_reset(ram)) == CGBL_SUCCESS)
            && ((result = cgbl_cartridge_mapper_reset()) == CGBL_SUCCESS))
        {
            cgbl_cartridge_hash_reset();
            cgbl_cartridge_title_reset();
        }
    }
    else
    {
        cartridge.mapper = &MAPPER[0].mapper;
        snprintf(cartridge.title, CGBL_LENGTH(cartridge.title), "UNDEFINED");
    }
    return result;
}

uint16_t cgbl_cartridge_rom_count(void)
{
    return cartridge.rom.count;
}

uint8_t cgbl_cartridge_rom_read(uint16_t bank, uint16_t address)
{
    return cartridge.rom.data[(bank * CGBL_CARTRIDGE_ROM_WIDTH) + address];
}

void cgbl_cartridge_step(void)
{
    if (!cartridge.clock.delay)
    {
        if (cartridge.ram.clock && !cartridge.ram.clock->day.halt)
        {
            if (++cartridge.ram.clock->second.counter == 60)
            {
                cartridge.ram.clock->second.counter = 0;
                if (++cartridge.ram.clock->minute.counter == 60)
                {
                    cartridge.ram.clock->minute.counter = 0;
                    if (++cartridge.ram.clock->hour.counter == 24)
                    {
                        cartridge.ram.clock->hour.counter = 0;
                        if ((cartridge.ram.clock->day.carry = (cartridge.ram.clock->day.counter == 511)))
                        {
                            cartridge.ram.clock->day.counter = 0;
                        }
                        else
                        {
                            ++cartridge.ram.clock->day.counter;
                        }
                    }
                }
            }
        }
        cartridge.clock.delay = 4213440;
    }
    --cartridge.clock.delay;
}

const char *cgbl_cartridge_title(void)
{
    return cartridge.title;
}

void cgbl_cartridge_write(uint16_t address, uint8_t data)
{
    if (cartridge.mapper->write)
    {
        cartridge.mapper->write(address, data);
    }
}
