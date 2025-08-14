/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "client.h"
#include "debug.h"
#include "processor.h"

typedef enum {
    CGBL_COMMAND_EXIT = 0,
    CGBL_COMMAND_HELP,
    CGBL_COMMAND_PROCESSOR,
    CGBL_COMMAND_RESET,
    CGBL_COMMAND_RUN,
    CGBL_COMMAND_STEP,
    CGBL_COMMAND_VERSION,
    CGBL_COMMAND_MAX,
} cgbl_command_e;

typedef enum {
    CGBL_MAPPER_0 = 0,
    CGBL_MAPPER_1,
    CGBL_MAPPER_2,
    CGBL_MAPPER_3,
    CGBL_MAPPER_5,
    CGBL_MAPPER_MAX,
} cgbl_mapper_e;

static const struct {
    uint8_t value;
    cgbl_mapper_e type;
} MAPPER[] = {
    { 0, CGBL_MAPPER_0 }, { 8, CGBL_MAPPER_0 }, { 9, CGBL_MAPPER_0 },
    { 1, CGBL_MAPPER_1 }, { 2, CGBL_MAPPER_1 }, { 3, CGBL_MAPPER_1 },
    { 5, CGBL_MAPPER_2 }, { 6, CGBL_MAPPER_2 },
    { 15, CGBL_MAPPER_3 }, { 16, CGBL_MAPPER_3 }, { 17, CGBL_MAPPER_3 }, { 18, CGBL_MAPPER_3 },
    { 19, CGBL_MAPPER_3 },
    { 25, CGBL_MAPPER_5 }, { 26, CGBL_MAPPER_5 }, { 27, CGBL_MAPPER_5 }, { 28, CGBL_MAPPER_5 },
    { 29, CGBL_MAPPER_5 }, { 30, CGBL_MAPPER_5 },
};

static const struct {
    uint8_t value;
    cgbl_mode_e type;
} MODE[] = {
    { 0x00, CGBL_MODE_DMG }, { 0x80, CGBL_MODE_DMG },
    { 0xC0, CGBL_MODE_CGB },
};

static const struct {
    char *name;
    char *description;
    char *usage;
    uint8_t length;
} OPTION[CGBL_COMMAND_MAX] = {
    { .name = "exit", .description = "Exit debug console", .usage = "exit", .length = 1, },
    { .name = "help", .description = "Display help information", .usage = "help", .length = 1, },
    { .name = "proc", .description = "Display processor information", .usage = "proc", .length = 1, },
    { .name = "rst", .description = "Reset bus", .usage = "rst", .length = 1, },
    { .name = "run", .description = "Run to breakpoint", .usage = "run  [breakpoint]", .length = 2, },
    { .name = "step", .description = "Step to next instruction", .usage = "step [breakpoint]", .length = 2, },
    { .name = "ver", .description = "Display version information", .usage = "ver", .length = 1, },
};

static const uint16_t RAM[] = {
    1, 1, 1, 4, 16, 8,
};

static const uint16_t ROM[] = {
    2, 4, 8, 16, 32, 64, 128, 256, 512,
};

static struct {
    char prompt[10];
    const char *path;
    cgbl_bank_t *ram;
    const cgbl_bank_t *rom;
} debug = {};

static cgbl_error_e cgbl_debug_command_exit(const char **const arguments, uint8_t length)
{
    fprintf(stdout, "Exiting\n");
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_help(const char **const arguments, uint8_t length)
{
    for (cgbl_command_e command = 0; command < CGBL_COMMAND_MAX; ++command)
    {
        char buffer[22] = {};
        snprintf(buffer, sizeof (buffer), "%s", OPTION[command].usage);
        for (uint32_t offset = strlen(buffer); offset < sizeof (buffer); ++offset)
        {
            buffer[offset] = (offset == (sizeof (buffer) - 1)) ? '\0' : ' ';
        }
        fprintf(stdout, "%s%s\n", buffer, OPTION[command].description);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_processor(const char **const arguments, uint8_t length)
{
    const cgbl_register_e reg[] = {
        CGBL_REGISTER_PC, CGBL_REGISTER_SP, CGBL_REGISTER_AF, CGBL_REGISTER_BC,
        CGBL_REGISTER_DE, CGBL_REGISTER_HL,
    };
    cgbl_error_e result = CGBL_SUCCESS;
    for (uint32_t index = 0; index < CGBL_LENGTH(reg); ++index)
    {
        cgbl_register_t value = {};
        if ((result = cgbl_processor_register_read(reg[index], &value)) != CGBL_SUCCESS)
        {
            break;
        }
        switch (reg[index])
        {
            case CGBL_REGISTER_AF:
                fprintf(stdout, "AF: %04X (A: %02X, F: %02X) [%c%c%c%c]\n", value.word, value.high, value.low,
                    value.carry ? 'C' : '-', value.half_carry ? 'H' : '-', value.negative ? 'N' : '-', value.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
                fprintf(stdout, "BC: %04X (B: %02X, C: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_DE:
                fprintf(stdout, "DE: %04X (D: %02X, E: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_HL:
                fprintf(stdout, "HL: %04X (H: %02X, L: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_PC:
                fprintf(stdout, "PC: %04X\n", value.word);
                break;
            case CGBL_REGISTER_SP:
                fprintf(stdout, "SP: %04X\n", value.word);
                break;
            default:
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_reset(const char **const arguments, uint8_t length)
{
    return cgbl_bus_reset(debug.rom, debug.ram);
}

static cgbl_error_e cgbl_debug_command_run(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_RUN].length)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
    for (;;)
    {
        if ((result = cgbl_client_poll()) != CGBL_SUCCESS)
        {
            if (result == CGBL_QUIT)
            {
                result = CGBL_SUCCESS;
            }
            break;
        }
        if ((result = cgbl_bus_run(breakpoint)) != CGBL_SUCCESS)
        {
            if (result != CGBL_QUIT)
            {
                if (result == CGBL_BREAKPOINT)
                {
                    fprintf(stdout, "Breakpoint: %04X\n", breakpoint);
                    result = CGBL_SUCCESS;
                }
                break;
            }
        }
        if ((result = cgbl_client_sync()) != CGBL_SUCCESS)
        {
            break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_step(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_STEP].length)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
    /* TODO: DISASSEMBLE INSTRUCTION */
    if ((result = cgbl_client_poll()) != CGBL_SUCCESS)
    {
        if (result == CGBL_QUIT)
        {
            result = CGBL_SUCCESS;
        }
        return result;
    }
    if ((result = cgbl_bus_step(breakpoint)) != CGBL_SUCCESS)
    {
        if (result == CGBL_BREAKPOINT)
        {
            fprintf(stdout, "Breakpoint: %04X\n", breakpoint);
            result = CGBL_SUCCESS;
        }
        return result;
    }
    if ((result = cgbl_client_sync()) != CGBL_SUCCESS)
    {
        return result;
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_version(const char **const arguments, uint8_t length)
{
    const cgbl_version_t *const version = cgbl_version();
    fprintf(stdout, "%u.%u-%x\n", version->major, version->minor, version->patch);
    return CGBL_SUCCESS;
}

static cgbl_error_e (*const COMMAND[CGBL_COMMAND_MAX])(const char **const arguments, uint8_t length) = {
    cgbl_debug_command_exit,
    cgbl_debug_command_help,
    cgbl_debug_command_processor,
    cgbl_debug_command_reset,
    cgbl_debug_command_run,
    cgbl_debug_command_step,
    cgbl_debug_command_version,
};

static char **cgbl_debug_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return NULL;
}

static cgbl_error_e cgbl_debug_header_checksum(void)
{
    uint8_t expected = 0, value = debug.rom->data[0x014D];
    for (uint32_t index = 0x0134; index <= 0x014C; ++index)
    {
        expected = expected - debug.rom->data[index] - 1;
    }
    if (expected != value)
    {
        return CGBL_ERROR("Mismatched checksum: %02X (expecting %02X)", value, expected);
    }
    fprintf(stdout, "014D | Checksum -- %02X\n", value);
    return CGBL_SUCCESS;
}

static void cgbl_debug_header_mapper(void)
{
    const char *mapper[CGBL_MAPPER_MAX] = {
        "MBC0", "MBC1", "MBC2", "MBC3", "MBC5",
    };
    uint8_t index = 0, value = debug.rom->data[0x0147];
    cgbl_mapper_e type = 0;
    for (; index < CGBL_LENGTH(MAPPER); ++index)
    {
        if (MAPPER[index].value == value)
        {
            type = MAPPER[index].type;
            break;
        }
    }
    fprintf(stdout, "0147 | Mapper   -- %02X (%s)\n", value, (index < CGBL_LENGTH(MAPPER)) ? mapper[type] : "????");
}

static void cgbl_debug_header_mode(void)
{
    const char *mode[CGBL_MODE_MAX] = {
        "DMG", "CGB",
    };
    uint8_t index = 0, value = debug.rom->data[0x0143];
    cgbl_mode_e type = 0;
    for (; index < CGBL_LENGTH(MODE); ++index)
    {
        if (MODE[index].value == value)
        {
            type = MODE[index].type;
            break;
        }
    }
    fprintf(stdout, "0143 | Mode     -- %02X (%s)\n", value, (index < CGBL_LENGTH(MODE)) ? mode[type] : "???");
}

static cgbl_error_e cgbl_debug_header_ram(void)
{
    uint32_t bank = 0;
    uint8_t value = debug.rom->data[0x0149];
    if (value >= CGBL_LENGTH(RAM))
    {
        return CGBL_ERROR("Invalid ram type: %u", value);
    }
    bank = RAM[value];
    fprintf(stdout, "0149 | Ram      -- %02X (%u banks, %u bytes)\n", value, bank, bank * 0x2000);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_header_rom(void)
{
    uint32_t bank = 0;
    uint8_t value = debug.rom->data[0x0148];
    if (value >= CGBL_LENGTH(ROM))
    {
        return CGBL_ERROR("Invalid rom type: %u", value);
    }
    bank = ROM[value];
    if ((bank * 0x4000) != debug.rom->length)
    {
        return CGBL_ERROR("Mismatched rom length: %u bytes (expecting %u bytes)", debug.rom->length, bank * 0x4000);
    }
    fprintf(stdout, "0148 | Rom      -- %02X (%u banks, %u bytes)\n", value, bank, bank * 0x4000);
    return CGBL_SUCCESS;
}

static void cgbl_debug_header_title(void)
{
    char title[12] = {};
    for (uint32_t index = 0; index < (sizeof (title) - 1); ++index)
    {
        char value = debug.rom->data[0x0134 + index];
        if (!value)
        {
            break;
        }
        title[index] = value;
    }
    if (!strlen(title))
    {
        strcpy(title, "UNDEFINED");
    }
    fprintf(stdout, "0134 | Title    -- %s\n", title);
}

static cgbl_error_e cgbl_debug_header(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    const cgbl_version_t *const version = cgbl_version();
    fprintf(stdout, "CGBL %u.%u-%x\n", version->major, version->minor, version->patch);
    fprintf(stdout, "Path: %s\n\n", debug.path);
    if (debug.rom && debug.rom->data)
    {
        cgbl_debug_header_title();
        cgbl_debug_header_mode();
        cgbl_debug_header_mapper();
        if ((result = cgbl_debug_header_rom()) != CGBL_SUCCESS)
        {
            return result;
        }
        if ((result = cgbl_debug_header_ram()) != CGBL_SUCCESS)
        {
            return result;
        }
        if ((result = cgbl_debug_header_checksum()) != CGBL_SUCCESS)
        {
            return result;
        }
    }
    return result;
}

static const char *cgbl_debug_prompt(void)
{
    cgbl_register_t reg = {};
    memset(debug.prompt, 0, sizeof (debug.prompt));
    if (cgbl_processor_register_read(CGBL_REGISTER_PC, &reg) == CGBL_SUCCESS)
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n%04X> ", reg.word);
    }
    else
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n\?\?\?\?> ");
    }
    return debug.prompt;
}

cgbl_error_e cgbl_debug_entry(const char *const path, const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    rl_attempted_completion_function = cgbl_debug_completion;
    memset(&debug, 0, sizeof (debug));
    debug.path = path;
    debug.ram = ram;
    debug.rom = rom;
    if ((result = cgbl_debug_header()) != CGBL_SUCCESS)
    {
        return result;
    }
    for (;;)
    {
        char *input = NULL;
        cgbl_command_e command = CGBL_COMMAND_MAX;
        if ((input = readline(cgbl_debug_prompt())) && strlen(input))
        {
            uint8_t length = 0;
            char *argument = NULL;
            const char *arguments[4] = {};
            add_history(input);
            argument = strtok(input, " ");
            while (argument)
            {
                if (length >= 4)
                {
                    break;
                }
                arguments[length++] = argument;
                argument = strtok(NULL, " ");
            }
            for (command = 0; command < CGBL_COMMAND_MAX; ++command)
            {
                if (!strcmp(input, OPTION[command].name))
                {
                    break;
                }
            }
            if (command >= CGBL_COMMAND_MAX)
            {
                fprintf(stderr, "Command unsupported: \'%s\'\n", input);
                fprintf(stderr, "Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if (length > OPTION[command].length)
            {
                fprintf(stderr, "Command usage: %s\n", OPTION[command].usage);
                fprintf(stderr, "Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((result = COMMAND[command](arguments, length)) == CGBL_FAILURE)
            {
                fprintf(stderr, "Command failed: \'%s\'\n", input);
            }
        }
        free(input);
        if ((command == CGBL_COMMAND_EXIT) && (result == CGBL_SUCCESS))
        {
            break;
        }
    }
    return result;
}
