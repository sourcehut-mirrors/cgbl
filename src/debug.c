/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "cartridge.h"
#include "client.h"
#include "debug.h"
#include "processor.h"

#define CGBL_TRACE_ERROR(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_ERROR, _FORMAT_, ##__VA_ARGS__)
#define CGBL_TRACE_WARNING(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_WARNING, _FORMAT_, ##__VA_ARGS__)
#define CGBL_TRACE_INFORMATION(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_INFORMATION, _FORMAT_, ##__VA_ARGS__)

typedef enum {
    CGBL_COMMAND_EXIT = 0,
    CGBL_COMMAND_CARTRIDGE,
    CGBL_COMMAND_CLOCK_LATCH,
    CGBL_COMMAND_CLOCK_READ,
    CGBL_COMMAND_CLOCK_WRITE,
    CGBL_COMMAND_DISASSEMBLE,
    CGBL_COMMAND_HELP,
    CGBL_COMMAND_INTERRUPT,
    CGBL_COMMAND_MEMORY_READ,
    CGBL_COMMAND_MEMORY_WRITE,
    CGBL_COMMAND_PROCESSOR,
    CGBL_COMMAND_REGISTER_READ,
    CGBL_COMMAND_REGISTER_WRITE,
    CGBL_COMMAND_RESET,
    CGBL_COMMAND_RUN,
    CGBL_COMMAND_STEP,
    CGBL_COMMAND_VERSION,
    CGBL_COMMAND_MAX,
} cgbl_command_e;

typedef enum {
    CGBL_LEVEL_ERROR = 0,
    CGBL_LEVEL_WARNING,
    CGBL_LEVEL_INFORMATION,
    CGBL_LEVEL_PROMPT,
    CGBL_LEVEL_MAX,
} cgbl_level_e;

typedef enum {
    CGBL_OPERAND_NONE = 0,
    CGBL_OPERAND_BYTE,
    CGBL_OPERAND_WORD,
    CGBL_OPERAND_MAX,
} cgbl_operand_e;

static const char *CLOCK[CGBL_CLOCK_MAX] = {
    "", "sec", "min", "hr",
    "dayl", "dayh",
};

static const struct
{
    const char *format;
    cgbl_operand_e operand;
}
INSTRUCTION[][CGBL_INSTRUCTION_MAX] =
{
    {
        { "nop", CGBL_OPERAND_NONE, }, { "ld bc,$%04X", CGBL_OPERAND_WORD, }, { "ld (bc),a", CGBL_OPERAND_NONE, }, { "inc bc", CGBL_OPERAND_NONE, },
        { "inc b", CGBL_OPERAND_NONE, }, { "dec b", CGBL_OPERAND_NONE, }, { "ld b,$%02X", CGBL_OPERAND_BYTE, }, { "rlca", CGBL_OPERAND_NONE, },
        { "ld ($%04X),sp", CGBL_OPERAND_WORD, }, { "add hl,bc", CGBL_OPERAND_NONE, }, { "ld a,(bc)", CGBL_OPERAND_NONE, }, { "dec bc", CGBL_OPERAND_NONE, },
        { "inc c", CGBL_OPERAND_NONE, }, { "dec c", CGBL_OPERAND_NONE, }, { "ld c,$%02X", CGBL_OPERAND_BYTE, }, { "rrca", CGBL_OPERAND_NONE, },
        { "stop $%02X", CGBL_OPERAND_BYTE, }, { "ld de,$%04X", CGBL_OPERAND_WORD, }, { "ld (de),a", CGBL_OPERAND_NONE, }, { "inc de", CGBL_OPERAND_NONE, },
        { "inc d", CGBL_OPERAND_NONE, }, { "dec d", CGBL_OPERAND_NONE, }, { "ld d,$%02X", CGBL_OPERAND_BYTE, }, { "rla", CGBL_OPERAND_NONE, },
        { "jr $%02X", CGBL_OPERAND_BYTE, }, { "add hl,de", CGBL_OPERAND_NONE, }, { "ld a,(de)", CGBL_OPERAND_NONE, }, { "dec de", CGBL_OPERAND_NONE, },
        { "inc e", CGBL_OPERAND_NONE, }, { "dec e", CGBL_OPERAND_NONE, }, { "ld e,$%02X", CGBL_OPERAND_BYTE, }, { "rra", CGBL_OPERAND_NONE, },
        { "jr nz,$%02X", CGBL_OPERAND_BYTE, }, { "ld hl,$%04X", CGBL_OPERAND_WORD, }, { "ldi (hl),a", CGBL_OPERAND_NONE, }, { "inc hl", CGBL_OPERAND_NONE, },
        { "inc h", CGBL_OPERAND_NONE, }, { "dec h", CGBL_OPERAND_NONE, }, { "ld h,$%02X", CGBL_OPERAND_BYTE, }, { "daa", CGBL_OPERAND_NONE, },
        { "jr z,$%02X", CGBL_OPERAND_BYTE, }, { "add hl,hl", CGBL_OPERAND_NONE, }, { "ldi a,(hl)", CGBL_OPERAND_NONE, }, { "dec hl", CGBL_OPERAND_NONE, },
        { "inc l", CGBL_OPERAND_NONE, }, { "dec l", CGBL_OPERAND_NONE, }, { "ld l,$%02X", CGBL_OPERAND_BYTE, }, { "cpl", CGBL_OPERAND_NONE, },
        { "jr nc,$%02X", CGBL_OPERAND_BYTE, }, { "ld sp,$%04X", CGBL_OPERAND_WORD, }, { "ldd (hl),a", CGBL_OPERAND_NONE, }, { "inc sp", CGBL_OPERAND_NONE, },
        { "inc (hl)", CGBL_OPERAND_NONE, }, { "dec (hl)", CGBL_OPERAND_NONE, }, { "ld (hl),$%02X", CGBL_OPERAND_BYTE, }, { "scf", CGBL_OPERAND_NONE, },
        { "jr c,$%02X", CGBL_OPERAND_BYTE, }, { "add hl,sp", CGBL_OPERAND_NONE, }, { "ldd a,(hl)", CGBL_OPERAND_NONE, }, { "dec sp", CGBL_OPERAND_NONE, },
        { "inc a", CGBL_OPERAND_NONE, }, { "dec a", CGBL_OPERAND_NONE, }, { "ld a,$%02X", CGBL_OPERAND_BYTE, }, { "ccf", CGBL_OPERAND_NONE, },
        { "ld b,b", CGBL_OPERAND_NONE, }, { "ld b,c", CGBL_OPERAND_NONE, }, { "ld b,d", CGBL_OPERAND_NONE, }, { "ld b,e", CGBL_OPERAND_NONE, },
        { "ld b,h", CGBL_OPERAND_NONE, }, { "ld b,l", CGBL_OPERAND_NONE, }, { "ld b,(hl)", CGBL_OPERAND_NONE, }, { "ld b,a", CGBL_OPERAND_NONE, },
        { "ld c,b", CGBL_OPERAND_NONE, }, { "ld c,c", CGBL_OPERAND_NONE, }, { "ld c,d", CGBL_OPERAND_NONE, }, { "ld c,e", CGBL_OPERAND_NONE, },
        { "ld c,h", CGBL_OPERAND_NONE, }, { "ld c,l", CGBL_OPERAND_NONE, }, { "ld c,(hl)", CGBL_OPERAND_NONE, }, { "ld c,a", CGBL_OPERAND_NONE, },
        { "ld d,b", CGBL_OPERAND_NONE, }, { "ld d,c", CGBL_OPERAND_NONE, }, { "ld d,d", CGBL_OPERAND_NONE, }, { "ld d,e", CGBL_OPERAND_NONE, },
        { "ld d,h", CGBL_OPERAND_NONE, }, { "ld d,l", CGBL_OPERAND_NONE, }, { "ld d,(hl)", CGBL_OPERAND_NONE, }, { "ld d,a", CGBL_OPERAND_NONE, },
        { "ld e,b", CGBL_OPERAND_NONE, }, { "ld e,c", CGBL_OPERAND_NONE, }, { "ld e,d", CGBL_OPERAND_NONE, }, { "ld e,e", CGBL_OPERAND_NONE, },
        { "ld e,h", CGBL_OPERAND_NONE, }, { "ld e,l", CGBL_OPERAND_NONE, }, { "ld e,(hl)", CGBL_OPERAND_NONE, }, { "ld e,a", CGBL_OPERAND_NONE, },
        { "ld h,b", CGBL_OPERAND_NONE, }, { "ld h,c", CGBL_OPERAND_NONE, }, { "ld h,d", CGBL_OPERAND_NONE, }, { "ld h,e", CGBL_OPERAND_NONE, },
        { "ld h,h", CGBL_OPERAND_NONE, }, { "ld h,l", CGBL_OPERAND_NONE, }, { "ld h,(hl)", CGBL_OPERAND_NONE, }, { "ld h,a", CGBL_OPERAND_NONE, },
        { "ld l,b", CGBL_OPERAND_NONE, }, { "ld l,c", CGBL_OPERAND_NONE, }, { "ld l,d", CGBL_OPERAND_NONE, }, { "ld l,e", CGBL_OPERAND_NONE, },
        { "ld l,h", CGBL_OPERAND_NONE, }, { "ld l,l", CGBL_OPERAND_NONE, }, { "ld l,(hl)", CGBL_OPERAND_NONE, }, { "ld l,a", CGBL_OPERAND_NONE, },
        { "ld (hl),b", CGBL_OPERAND_NONE, }, { "ld (hl),c", CGBL_OPERAND_NONE, }, { "ld (hl),d", CGBL_OPERAND_NONE, }, { "ld (hl),e", CGBL_OPERAND_NONE, },
        { "ld (hl),h", CGBL_OPERAND_NONE, }, { "ld (hl),l", CGBL_OPERAND_NONE, }, { "halt", CGBL_OPERAND_NONE, }, { "ld (hl),a", CGBL_OPERAND_NONE, },
        { "ld a,b", CGBL_OPERAND_NONE, }, { "ld a,c", CGBL_OPERAND_NONE, }, { "ld a,d", CGBL_OPERAND_NONE, }, { "ld a,e", CGBL_OPERAND_NONE, },
        { "ld a,h", CGBL_OPERAND_NONE, }, { "ld a,l", CGBL_OPERAND_NONE, }, { "ld a,(hl)", CGBL_OPERAND_NONE, }, { "ld a,a", CGBL_OPERAND_NONE, },
        { "add a,b", CGBL_OPERAND_NONE, }, { "add a,c", CGBL_OPERAND_NONE, }, { "add a,d", CGBL_OPERAND_NONE, }, { "add a,e", CGBL_OPERAND_NONE, },
        { "add a,h", CGBL_OPERAND_NONE, }, { "add a,l", CGBL_OPERAND_NONE, }, { "add a,(hl)", CGBL_OPERAND_NONE, }, { "add a,a", CGBL_OPERAND_NONE, },
        { "adc a,b", CGBL_OPERAND_NONE, }, { "adc a,c", CGBL_OPERAND_NONE, }, { "adc a,d", CGBL_OPERAND_NONE, }, { "adc a,e", CGBL_OPERAND_NONE, },
        { "adc a,h", CGBL_OPERAND_NONE, }, { "adc a,l", CGBL_OPERAND_NONE, }, { "adc a,(hl)", CGBL_OPERAND_NONE, }, { "adc a,a", CGBL_OPERAND_NONE, },
        { "sub a,b", CGBL_OPERAND_NONE, }, { "sub a,c", CGBL_OPERAND_NONE, }, { "sub a,d", CGBL_OPERAND_NONE, }, { "sub a,e", CGBL_OPERAND_NONE, },
        { "sub a,h", CGBL_OPERAND_NONE, }, { "sub a,l", CGBL_OPERAND_NONE, }, { "sub a,(hl)", CGBL_OPERAND_NONE, }, { "sub a,a", CGBL_OPERAND_NONE, },
        { "sbc a,b", CGBL_OPERAND_NONE, }, { "sbc a,c", CGBL_OPERAND_NONE, }, { "sbc a,d", CGBL_OPERAND_NONE, }, { "sbc a,e", CGBL_OPERAND_NONE, },
        { "sbc a,h", CGBL_OPERAND_NONE, }, { "sbc a,l", CGBL_OPERAND_NONE, }, { "sbc a,(hl)", CGBL_OPERAND_NONE, }, { "sbc a,a", CGBL_OPERAND_NONE, },
        { "and a,b", CGBL_OPERAND_NONE, }, { "and a,c", CGBL_OPERAND_NONE, }, { "and a,d", CGBL_OPERAND_NONE, }, { "and a,e", CGBL_OPERAND_NONE, },
        { "and a,h", CGBL_OPERAND_NONE, }, { "and a,l", CGBL_OPERAND_NONE, }, { "and a,(hl)", CGBL_OPERAND_NONE, }, { "and a,a", CGBL_OPERAND_NONE, },
        { "xor a,b", CGBL_OPERAND_NONE, }, { "xor a,c", CGBL_OPERAND_NONE, }, { "xor a,d", CGBL_OPERAND_NONE, }, { "xor a,e", CGBL_OPERAND_NONE, },
        { "xor a,h", CGBL_OPERAND_NONE, }, { "xor a,l", CGBL_OPERAND_NONE, }, { "xor a,(hl)", CGBL_OPERAND_NONE, }, { "xor a,a", CGBL_OPERAND_NONE, },
        { "or a,b", CGBL_OPERAND_NONE, }, { "or a,c", CGBL_OPERAND_NONE, }, { "or a,d", CGBL_OPERAND_NONE, }, { "or a,e", CGBL_OPERAND_NONE, },
        { "or a,h", CGBL_OPERAND_NONE, }, { "or a,l", CGBL_OPERAND_NONE, }, { "or a,(hl)", CGBL_OPERAND_NONE, }, { "or a,a", CGBL_OPERAND_NONE, },
        { "cp a,b", CGBL_OPERAND_NONE, }, { "cp a,c", CGBL_OPERAND_NONE, }, { "cp a,d", CGBL_OPERAND_NONE, }, { "cp a,e", CGBL_OPERAND_NONE, },
        { "cp a,h", CGBL_OPERAND_NONE, }, { "cp a,l", CGBL_OPERAND_NONE, }, { "cp a,(hl)", CGBL_OPERAND_NONE, }, { "cp a,a", CGBL_OPERAND_NONE, },
        { "ret nz", CGBL_OPERAND_NONE, }, { "pop bc", CGBL_OPERAND_NONE, }, { "jp nz,$%04X", CGBL_OPERAND_WORD, }, { "jp $%04X", CGBL_OPERAND_WORD, },
        { "call nz,$%04X", CGBL_OPERAND_WORD, }, { "push bc", CGBL_OPERAND_NONE, }, { "and a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $00", CGBL_OPERAND_NONE, },
        { "ret z", CGBL_OPERAND_NONE, }, { "ret", CGBL_OPERAND_NONE, }, { "jp z,$%04X", CGBL_OPERAND_WORD, }, { "???", CGBL_OPERAND_NONE, },
        { "call z,$%04X", CGBL_OPERAND_WORD, }, { "call $%04X", CGBL_OPERAND_WORD, }, { "adc a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $08", CGBL_OPERAND_NONE, },
        { "ret nc", CGBL_OPERAND_NONE, }, { "pop de", CGBL_OPERAND_NONE, }, { "jp nc,$%04X", CGBL_OPERAND_WORD, }, { "???", CGBL_OPERAND_NONE, },
        { "call nz,$%04X", CGBL_OPERAND_WORD, }, { "push de", CGBL_OPERAND_NONE, }, { "sub a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $10", CGBL_OPERAND_NONE, },
        { "ret c", CGBL_OPERAND_NONE, }, { "reti", CGBL_OPERAND_NONE, }, { "jp c,$%04X", CGBL_OPERAND_WORD, }, { "???", CGBL_OPERAND_NONE, },
        { "call c,$%04X", CGBL_OPERAND_WORD, }, { "???", CGBL_OPERAND_NONE, }, { "sbc a,$%02X", CGBL_OPERAND_WORD, }, { "rst $18", CGBL_OPERAND_NONE, },
        { "ld ($FF00+$%02X),a", CGBL_OPERAND_BYTE, }, { "pop hl", CGBL_OPERAND_NONE, }, { "ld ($FF00+c),a", CGBL_OPERAND_NONE, }, { "???", CGBL_OPERAND_NONE, },
        { "???", CGBL_OPERAND_NONE, }, { "push hl", CGBL_OPERAND_NONE, }, { "and a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $20", CGBL_OPERAND_NONE, },
        { "add sp,$%02X", CGBL_OPERAND_BYTE, }, { "jp hl", CGBL_OPERAND_NONE, }, { "ld ($%04X),a", CGBL_OPERAND_WORD, }, { "???", CGBL_OPERAND_NONE, },
        { "???", CGBL_OPERAND_NONE, }, { "???", CGBL_OPERAND_NONE, }, { "xor a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $28", CGBL_OPERAND_NONE, },
        { "ld a,($FF00+$%02X)", CGBL_OPERAND_BYTE, }, { "pop af", CGBL_OPERAND_NONE, }, { "ld a,($FF00+c)", CGBL_OPERAND_NONE, }, { "di", CGBL_OPERAND_NONE, },
        { "???", CGBL_OPERAND_NONE, }, { "push af", CGBL_OPERAND_NONE, }, { "or a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $30", CGBL_OPERAND_NONE, },
        { "ld hl,sp+$%02X", CGBL_OPERAND_BYTE, }, { "ld sp,hl", CGBL_OPERAND_NONE, }, { "ld a,($%04X)", CGBL_OPERAND_WORD, }, { "ei", CGBL_OPERAND_BYTE, },
        { "???", CGBL_OPERAND_NONE, }, { "???", CGBL_OPERAND_NONE, }, { "cp a,$%02X", CGBL_OPERAND_BYTE, }, { "rst $38", CGBL_OPERAND_NONE, },
    },
    {
        { "rlc b", CGBL_OPERAND_NONE, }, { "rlc c", CGBL_OPERAND_NONE, }, { "rlc d", CGBL_OPERAND_NONE, }, { "rlc e", CGBL_OPERAND_NONE, },
        { "rlc h", CGBL_OPERAND_NONE, }, { "rlc l", CGBL_OPERAND_NONE, }, { "rlc (hl)", CGBL_OPERAND_NONE, }, { "rlc a", CGBL_OPERAND_NONE, },
        { "rrc b", CGBL_OPERAND_NONE, }, { "rrc c", CGBL_OPERAND_NONE, }, { "rrc d", CGBL_OPERAND_NONE, }, { "rrc e", CGBL_OPERAND_NONE, },
        { "rrc h", CGBL_OPERAND_NONE, }, { "rrc l", CGBL_OPERAND_NONE, }, { "rrc (hl)", CGBL_OPERAND_NONE, }, { "rrc a", CGBL_OPERAND_NONE, },
        { "rl b", CGBL_OPERAND_NONE, }, { "rl c", CGBL_OPERAND_NONE, }, { "rl d", CGBL_OPERAND_NONE, }, { "rl e", CGBL_OPERAND_NONE, },
        { "rl h", CGBL_OPERAND_NONE, }, { "rl l", CGBL_OPERAND_NONE, }, { "rl (hl)", CGBL_OPERAND_NONE, }, { "rl a", CGBL_OPERAND_NONE, },
        { "rr b", CGBL_OPERAND_NONE, }, { "rr c", CGBL_OPERAND_NONE, }, { "rr d", CGBL_OPERAND_NONE, }, { "rr e", CGBL_OPERAND_NONE, },
        { "rr h", CGBL_OPERAND_NONE, }, { "rr l", CGBL_OPERAND_NONE, }, { "rr (hl)", CGBL_OPERAND_NONE, }, { "rr a", CGBL_OPERAND_NONE, },
        { "sla b", CGBL_OPERAND_NONE, }, { "sla c", CGBL_OPERAND_NONE, }, { "sla d", CGBL_OPERAND_NONE, }, { "sla e", CGBL_OPERAND_NONE, },
        { "sla h", CGBL_OPERAND_NONE, }, { "sla l", CGBL_OPERAND_NONE, }, { "sla (hl)", CGBL_OPERAND_NONE, }, { "sla a", CGBL_OPERAND_NONE, },
        { "sra b", CGBL_OPERAND_NONE, }, { "sra c", CGBL_OPERAND_NONE, }, { "sra d", CGBL_OPERAND_NONE, }, { "sra e", CGBL_OPERAND_NONE, },
        { "sra h", CGBL_OPERAND_NONE, }, { "sra l", CGBL_OPERAND_NONE, }, { "sra (hl)", CGBL_OPERAND_NONE, }, { "sra a", CGBL_OPERAND_NONE, },
        { "swap b", CGBL_OPERAND_NONE, }, { "swap c", CGBL_OPERAND_NONE, }, { "swap d", CGBL_OPERAND_NONE, }, { "swap e", CGBL_OPERAND_NONE, },
        { "swap h", CGBL_OPERAND_NONE, }, { "swap l", CGBL_OPERAND_NONE, }, { "swap (hl)", CGBL_OPERAND_NONE, }, { "swap a", CGBL_OPERAND_NONE, },
        { "srl b", CGBL_OPERAND_NONE, }, { "srl c", CGBL_OPERAND_NONE, }, { "srl d", CGBL_OPERAND_NONE, }, { "srl e", CGBL_OPERAND_NONE, },
        { "srl h", CGBL_OPERAND_NONE, }, { "srl l", CGBL_OPERAND_NONE, }, { "srl (hl)", CGBL_OPERAND_NONE, }, { "srl a", CGBL_OPERAND_NONE, },
        { "bit 0,b", CGBL_OPERAND_NONE, }, { "bit 0,c", CGBL_OPERAND_NONE, }, { "bit 0,d", CGBL_OPERAND_NONE, }, { "bit 0,e", CGBL_OPERAND_NONE, },
        { "bit 0,h", CGBL_OPERAND_NONE, }, { "bit 0,l", CGBL_OPERAND_NONE, }, { "bit 0,(hl)", CGBL_OPERAND_NONE, }, { "bit 0,a", CGBL_OPERAND_NONE, },
        { "bit 1,b", CGBL_OPERAND_NONE, }, { "bit 1,c", CGBL_OPERAND_NONE, }, { "bit 1,d", CGBL_OPERAND_NONE, }, { "bit 1,e", CGBL_OPERAND_NONE, },
        { "bit 1,h", CGBL_OPERAND_NONE, }, { "bit 1,l", CGBL_OPERAND_NONE, }, { "bit 1,(hl)", CGBL_OPERAND_NONE, }, { "bit 1,a", CGBL_OPERAND_NONE, },
        { "bit 2,b", CGBL_OPERAND_NONE, }, { "bit 2,c", CGBL_OPERAND_NONE, }, { "bit 2,d", CGBL_OPERAND_NONE, }, { "bit 2,e", CGBL_OPERAND_NONE, },
        { "bit 2,h", CGBL_OPERAND_NONE, }, { "bit 2,l", CGBL_OPERAND_NONE, }, { "bit 2,(hl)", CGBL_OPERAND_NONE, }, { "bit 2,a", CGBL_OPERAND_NONE, },
        { "bit 3,b", CGBL_OPERAND_NONE, }, { "bit 3,c", CGBL_OPERAND_NONE, }, { "bit 3,d", CGBL_OPERAND_NONE, }, { "bit 3,e", CGBL_OPERAND_NONE, },
        { "bit 3,h", CGBL_OPERAND_NONE, }, { "bit 3,l", CGBL_OPERAND_NONE, }, { "bit 3,(hl)", CGBL_OPERAND_NONE, }, { "bit 3,a", CGBL_OPERAND_NONE, },
        { "bit 4,b", CGBL_OPERAND_NONE, }, { "bit 4,c", CGBL_OPERAND_NONE, }, { "bit 4,d", CGBL_OPERAND_NONE, }, { "bit 4,e", CGBL_OPERAND_NONE, },
        { "bit 4,h", CGBL_OPERAND_NONE, }, { "bit 4,l", CGBL_OPERAND_NONE, }, { "bit 4,(hl)", CGBL_OPERAND_NONE, }, { "bit 4,a", CGBL_OPERAND_NONE, },
        { "bit 5,b", CGBL_OPERAND_NONE, }, { "bit 5,c", CGBL_OPERAND_NONE, }, { "bit 5,d", CGBL_OPERAND_NONE, }, { "bit 5,e", CGBL_OPERAND_NONE, },
        { "bit 5,h", CGBL_OPERAND_NONE, }, { "bit 5,l", CGBL_OPERAND_NONE, }, { "bit 5,(hl)", CGBL_OPERAND_NONE, }, { "bit 5,a", CGBL_OPERAND_NONE, },
        { "bit 6,b", CGBL_OPERAND_NONE, }, { "bit 6,c", CGBL_OPERAND_NONE, }, { "bit 6,d", CGBL_OPERAND_NONE, }, { "bit 6,e", CGBL_OPERAND_NONE, },
        { "bit 6,h", CGBL_OPERAND_NONE, }, { "bit 6,l", CGBL_OPERAND_NONE, }, { "bit 6,(hl)", CGBL_OPERAND_NONE, }, { "bit 6,a", CGBL_OPERAND_NONE, },
        { "bit 7,b", CGBL_OPERAND_NONE, }, { "bit 7,c", CGBL_OPERAND_NONE, }, { "bit 7,d", CGBL_OPERAND_NONE, }, { "bit 7,e", CGBL_OPERAND_NONE, },
        { "bit 7,h", CGBL_OPERAND_NONE, }, { "bit 7,l", CGBL_OPERAND_NONE, }, { "bit 7,(hl)", CGBL_OPERAND_NONE, }, { "bit 7,a", CGBL_OPERAND_NONE, },
        { "res 0,b", CGBL_OPERAND_NONE, }, { "res 0,c", CGBL_OPERAND_NONE, }, { "res 0,d", CGBL_OPERAND_NONE, }, { "res 0,e", CGBL_OPERAND_NONE, },
        { "res 0,h", CGBL_OPERAND_NONE, }, { "res 0,l", CGBL_OPERAND_NONE, }, { "res 0,(hl)", CGBL_OPERAND_NONE, }, { "res 0,a", CGBL_OPERAND_NONE, },
        { "res 1,b", CGBL_OPERAND_NONE, }, { "res 1,c", CGBL_OPERAND_NONE, }, { "res 1,d", CGBL_OPERAND_NONE, }, { "res 1,e", CGBL_OPERAND_NONE, },
        { "res 1,h", CGBL_OPERAND_NONE, }, { "res 1,l", CGBL_OPERAND_NONE, }, { "res 1,(hl)", CGBL_OPERAND_NONE, }, { "res 1,a", CGBL_OPERAND_NONE, },
        { "res 2,b", CGBL_OPERAND_NONE, }, { "res 2,c", CGBL_OPERAND_NONE, }, { "res 2,d", CGBL_OPERAND_NONE, }, { "res 2,e", CGBL_OPERAND_NONE, },
        { "res 2,h", CGBL_OPERAND_NONE, }, { "res 2,l", CGBL_OPERAND_NONE, }, { "res 2,(hl)", CGBL_OPERAND_NONE, }, { "res 2,a", CGBL_OPERAND_NONE, },
        { "res 3,b", CGBL_OPERAND_NONE, }, { "res 3,c", CGBL_OPERAND_NONE, }, { "res 3,d", CGBL_OPERAND_NONE, }, { "res 3,e", CGBL_OPERAND_NONE, },
        { "res 3,h", CGBL_OPERAND_NONE, }, { "res 3,l", CGBL_OPERAND_NONE, }, { "res 3,(hl)", CGBL_OPERAND_NONE, }, { "res 3,a", CGBL_OPERAND_NONE, },
        { "res 4,b", CGBL_OPERAND_NONE, }, { "res 4,c", CGBL_OPERAND_NONE, }, { "res 4,d", CGBL_OPERAND_NONE, }, { "res 4,e", CGBL_OPERAND_NONE, },
        { "res 4,h", CGBL_OPERAND_NONE, }, { "res 4,l", CGBL_OPERAND_NONE, }, { "res 4,(hl)", CGBL_OPERAND_NONE, }, { "res 4,a", CGBL_OPERAND_NONE, },
        { "res 5,b", CGBL_OPERAND_NONE, }, { "res 5,c", CGBL_OPERAND_NONE, }, { "res 5,d", CGBL_OPERAND_NONE, }, { "res 5,e", CGBL_OPERAND_NONE, },
        { "res 5,h", CGBL_OPERAND_NONE, }, { "res 5,l", CGBL_OPERAND_NONE, }, { "res 5,(hl)", CGBL_OPERAND_NONE, }, { "res 5,a", CGBL_OPERAND_NONE, },
        { "res 6,b", CGBL_OPERAND_NONE, }, { "res 6,c", CGBL_OPERAND_NONE, }, { "res 6,d", CGBL_OPERAND_NONE, }, { "res 6,e", CGBL_OPERAND_NONE, },
        { "res 6,h", CGBL_OPERAND_NONE, }, { "res 6,l", CGBL_OPERAND_NONE, }, { "res 6,(hl)", CGBL_OPERAND_NONE, }, { "res 6,a", CGBL_OPERAND_NONE, },
        { "res 7,b", CGBL_OPERAND_NONE, }, { "res 7,c", CGBL_OPERAND_NONE, }, { "res 7,d", CGBL_OPERAND_NONE, }, { "res 7,e", CGBL_OPERAND_NONE, },
        { "res 7,h", CGBL_OPERAND_NONE, }, { "res 7,l", CGBL_OPERAND_NONE, }, { "res 7,(hl)", CGBL_OPERAND_NONE, }, { "res 7,a", CGBL_OPERAND_NONE, },
        { "set 0,b", CGBL_OPERAND_NONE, }, { "set 0,c", CGBL_OPERAND_NONE, }, { "set 0,d", CGBL_OPERAND_NONE, }, { "set 0,e", CGBL_OPERAND_NONE, },
        { "set 0,h", CGBL_OPERAND_NONE, }, { "set 0,l", CGBL_OPERAND_NONE, }, { "set 0,(hl)", CGBL_OPERAND_NONE, }, { "set 0,a", CGBL_OPERAND_NONE, },
        { "set 1,b", CGBL_OPERAND_NONE, }, { "set 1,c", CGBL_OPERAND_NONE, }, { "set 1,d", CGBL_OPERAND_NONE, }, { "set 1,e", CGBL_OPERAND_NONE, },
        { "set 1,h", CGBL_OPERAND_NONE, }, { "set 1,l", CGBL_OPERAND_NONE, }, { "set 1,(hl)", CGBL_OPERAND_NONE, }, { "set 1,a", CGBL_OPERAND_NONE, },
        { "set 2,b", CGBL_OPERAND_NONE, }, { "set 2,c", CGBL_OPERAND_NONE, }, { "set 2,d", CGBL_OPERAND_NONE, }, { "set 2,e", CGBL_OPERAND_NONE, },
        { "set 2,h", CGBL_OPERAND_NONE, }, { "set 2,l", CGBL_OPERAND_NONE, }, { "set 2,(hl)", CGBL_OPERAND_NONE, }, { "set 2,a", CGBL_OPERAND_NONE, },
        { "set 3,b", CGBL_OPERAND_NONE, }, { "set 3,c", CGBL_OPERAND_NONE, }, { "set 3,d", CGBL_OPERAND_NONE, }, { "set 3,e", CGBL_OPERAND_NONE, },
        { "set 3,h", CGBL_OPERAND_NONE, }, { "set 3,l", CGBL_OPERAND_NONE, }, { "set 3,(hl)", CGBL_OPERAND_NONE, }, { "set 3,a", CGBL_OPERAND_NONE, },
        { "set 4,b", CGBL_OPERAND_NONE, }, { "set 4,c", CGBL_OPERAND_NONE, }, { "set 4,d", CGBL_OPERAND_NONE, }, { "set 4,e", CGBL_OPERAND_NONE, },
        { "set 4,h", CGBL_OPERAND_NONE, }, { "set 4,l", CGBL_OPERAND_NONE, }, { "set 4,(hl)", CGBL_OPERAND_NONE, }, { "set 4,a", CGBL_OPERAND_NONE, },
        { "set 5,b", CGBL_OPERAND_NONE, }, { "set 5,c", CGBL_OPERAND_NONE, }, { "set 5,d", CGBL_OPERAND_NONE, }, { "set 5,e", CGBL_OPERAND_NONE, },
        { "set 5,h", CGBL_OPERAND_NONE, }, { "set 5,l", CGBL_OPERAND_NONE, }, { "set 5,(hl)", CGBL_OPERAND_NONE, }, { "set 5,a", CGBL_OPERAND_NONE, },
        { "set 6,b", CGBL_OPERAND_NONE, }, { "set 6,c", CGBL_OPERAND_NONE, }, { "set 6,d", CGBL_OPERAND_NONE, }, { "set 6,e", CGBL_OPERAND_NONE, },
        { "set 6,h", CGBL_OPERAND_NONE, }, { "set 6,l", CGBL_OPERAND_NONE, }, { "set 6,(hl)", CGBL_OPERAND_NONE, }, { "set 6,a", CGBL_OPERAND_NONE, },
        { "set 7,b", CGBL_OPERAND_NONE, }, { "set 7,c", CGBL_OPERAND_NONE, }, { "set 7,d", CGBL_OPERAND_NONE, }, { "set 7,e", CGBL_OPERAND_NONE, },
        { "set 7,h", CGBL_OPERAND_NONE, }, { "set 7,l", CGBL_OPERAND_NONE, }, { "set 7,(hl)", CGBL_OPERAND_NONE, }, { "set 7,a", CGBL_OPERAND_NONE, },
    },
};

static const char *INTERRUPT[CGBL_INTERRUPT_MAX] = {
    "vblk", "lcdc", "tmr", "ser",
    "joy",
};

static const struct {
    uint8_t type;
    const char *name;
} MAPPER[] = {
    { 0, "MBC0" }, { 8, "MBC0" }, { 9, "MBC0" },
    { 1, "MBC1" }, { 2, "MBC1" }, { 3, "MBC1" },
    { 5, "MBC2" }, { 6, "MBC2" },
    { 15, "MBC3" }, { 16, "MBC3" }, { 17, "MBC3" }, { 18, "MBC3" },
    { 19, "MBC3" },
    { 25, "MBC5" }, { 26, "MBC5" }, { 27, "MBC5" }, { 28, "MBC5" },
    { 29, "MBC5" }, { 30, "MBC5" },
};

static const struct {
    uint8_t type;
    const char *name;
} MODE[] = {
    { 0x00, "DMG" }, { 0x80, "DMG" },
    { 0xC0, "CGB" },
};

static const uint16_t RAM[] = {
    1, 1, 1, 4, 16, 8,
};

static const uint16_t ROM[] = {
    2, 4, 8, 16, 32, 64, 128, 256, 512,
};

static const struct {
    const char *name;
    const char *description;
    const char *usage;
    uint8_t min;
    uint8_t max;
} OPTION[CGBL_COMMAND_MAX] = {
    { .name = "exit", .description = "Exit debug console", .usage = "", .min = 1, .max = 1, },
    { .name = "cart", .description = "Display cartridge information", .usage = "", .min = 1, .max = 1, },
    { .name = "clkl", .description = "Latch clock", .usage = "", .min = 1, .max = 1, },
    { .name = "clkr", .description = "Read data from clock", .usage = "clk", .min = 2, .max = 2, },
    { .name = "clkw", .description = "Write data to clock", .usage = "clk data", .min = 3, .max = 3, },
    { .name = "dasm", .description = "Disassemble instructions", .usage = "addr [off]", .min = 2, .max = 3, },
    { .name = "help", .description = "Display help information", .usage = "", .min = 1, .max = 1, },
    { .name = "itr", .description = "Interrupt bus", .usage = "int", .min = 2, .max = 2, },
    { .name = "memr", .description = "Read data from memory", .usage = "addr [off]", .min = 2, .max = 3, },
    { .name = "memw", .description = "Write data to memory", .usage = "addr data [off]", .min = 3, .max = 4, },
    { .name = "proc", .description = "Display processor information", .usage = "", .min = 1, .max = 1, },
    { .name = "regr", .description = "Read data from register", .usage = "reg", .min = 2, .max = 2, },
    { .name = "regw", .description = "Write data to register", .usage = "reg data", .min = 3, .max = 3, },
    { .name = "rst", .description = "Reset bus", .usage = "", .min = 1, .max = 1, },
    { .name = "run", .description = "Run to breakpoint", .usage = "[bp]", .min = 1, .max = 2, },
    { .name = "step", .description = "Step to next instruction", .usage = "[bp]", .min = 1, .max = 2, },
    { .name = "ver", .description = "Display version information", .usage = "", .min = 1, .max = 1, },
};

static const char *REGISTER[CGBL_REGISTER_MAX] = {
    "a", "af", "b", "bc",
    "c", "d", "de", "e",
    "f", "h", "hl", "l",
    "pc", "sp",
};

static const struct {
    const char *begin;
    const char *end;
} TRACE[CGBL_LEVEL_MAX] = {
    { "\x1B[31m", "\x1B[m", },
    { "\x1B[93m", "\x1B[m", },
    { "\x1B[0m", "\x1B[m", },
    { "\x1B[32m", "\x1B[m", },
};

static struct {
    char prompt[32];
    const char *path;
    cgbl_bank_t *ram;
    const cgbl_bank_t *rom;
} debug = {};

static inline void cgbl_debug_trace(cgbl_level_e level, const char *const format, ...)
{
    char buffer[128] = {};
    FILE *stream = NULL;
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(buffer, sizeof (buffer), format, arguments);
    va_end(arguments);
    switch (level)
    {
        case CGBL_LEVEL_ERROR:
        case CGBL_LEVEL_WARNING:
            stream = stderr;
            break;
        default:
            stream = stdout;
            break;
    }
    fprintf(stream, "%s%s%s", TRACE[level].begin, buffer, TRACE[level].end);
}

static inline cgbl_clock_e cgbl_debug_clock(const char *const name)
{
    cgbl_clock_e result = CGBL_CLOCK_SECOND;
    for (; result < CGBL_CLOCK_MAX; ++result)
    {
        if (!strcmp(name, CLOCK[result]))
        {
            break;
        }
    }
    return result;
}

static inline void cgbl_debug_disassemble_instruction(uint16_t *address, bool verbose)
{
    uint8_t data[4] = {}, length = 0;
    const char *format = NULL;
    cgbl_operand_e operand = CGBL_OPERAND_NONE;
    if (verbose)
    {
        CGBL_TRACE_INFORMATION("%04X |", *address);
    }
    data[length++] = cgbl_bus_read((*address)++);
    if (data[length - 1] == CGBL_INSTRUCTION_PREFIX)
    {
        data[length++] = cgbl_bus_read((*address)++);
        format = INSTRUCTION[1][data[length - 1]].format;
        operand = INSTRUCTION[1][data[length - 1]].operand;
    }
    else
    {
        format = INSTRUCTION[0][data[length - 1]].format;
        operand = INSTRUCTION[0][data[length - 1]].operand;
    }
    switch (operand)
    {
        case CGBL_OPERAND_BYTE:
            data[length++] = cgbl_bus_read((*address)++);
            break;
        case CGBL_OPERAND_WORD:
            data[length++] = cgbl_bus_read((*address)++);
            data[length++] = cgbl_bus_read((*address)++);
            break;
        default:
            break;
    }
    if (verbose)
    {
        for (uint8_t index = 0; index < CGBL_LENGTH(data); ++index)
        {
            if (index < length)
            {
                CGBL_TRACE_INFORMATION(" %02X", data[index]);
            }
            else
            {
                CGBL_TRACE_INFORMATION("   ");
            }
        }
    }
    switch (operand)
    {
        case CGBL_OPERAND_BYTE:
            CGBL_TRACE_INFORMATION(format, data[length - 1]);
            break;
        case CGBL_OPERAND_WORD:
            CGBL_TRACE_INFORMATION(format, *(uint16_t *)&data[length - 2]);
            break;
        default:
            CGBL_TRACE_INFORMATION(format);
            break;
    }
    CGBL_TRACE_INFORMATION("\n");
}

static inline void cgbl_debug_disassemble(uint16_t address, uint32_t offset)
{
    if (offset > 1)
    {
        CGBL_TRACE_INFORMATION("[%04X, %u instructions]\n\n", address, offset);
        for (uint32_t index = 0; index < offset; ++index)
        {
            cgbl_debug_disassemble_instruction(&address, true);
        }
    }
    else
    {
        cgbl_debug_disassemble_instruction(&address, false);
    }
}

static inline cgbl_interrupt_e cgbl_debug_interrupt(const char *const name)
{
    cgbl_interrupt_e result = 0;
    for (; result < CGBL_INTERRUPT_MAX; ++result)
    {
        if (!strcmp(name, INTERRUPT[result]))
        {
            break;
        }
    }
    return result;
}

static inline void cgbl_debug_memory(uint16_t address, uint32_t offset)
{
    if (offset > 1)
    {
        char buffer[17] = {};
        uint32_t begin = address, end = (address + offset) & 0xFFFF;
        if (begin % 16)
        {
            begin -= begin % 16;
        }
        if (end % 16)
        {
            end += 16 - (end % 16);
        }
        CGBL_TRACE_INFORMATION("[%04X-%04X, %u bytes]\n\n      ", address, (address + offset - 1) & 0xFFFF, offset);
        for (uint32_t index = 0; index < 16; ++index)
        {
            CGBL_TRACE_INFORMATION(" %02X", index);
        }
        CGBL_TRACE_INFORMATION("\n      ");
        for (uint32_t index = 0; index < 16; ++index)
        {
            CGBL_TRACE_INFORMATION(" --");
        }
        for (uint32_t index = begin; index < end; ++index)
        {
            if (!(index % 16))
            {
                if (strlen(buffer))
                {
                    CGBL_TRACE_INFORMATION("   %s", buffer);
                    memset(buffer, 0, sizeof (buffer));
                }
                CGBL_TRACE_INFORMATION("\n%04X |", index);
            }
            if ((index >= address) && (index < ((address + offset) & 0xFFFF)))
            {
                uint8_t value = cgbl_bus_read(index);
                CGBL_TRACE_INFORMATION(" %02X", value);
                buffer[index % 16] = (!isprint(value) || isspace(value)) ? '.' : value;
            }
            else
            {
                CGBL_TRACE_INFORMATION(" --");
                buffer[index % 16] = '.';
            }
        }
        if (strlen(buffer))
        {
            CGBL_TRACE_INFORMATION("   %s", buffer);
            memset(buffer, 0, sizeof (buffer));
        }
        CGBL_TRACE_INFORMATION("\n");
    }
    else
    {
        CGBL_TRACE_INFORMATION("%02X\n", cgbl_bus_read(address));
    }
}

static inline cgbl_register_e cgbl_debug_register(const char *const name)
{
    cgbl_register_e result = 0;
    for (; result < CGBL_REGISTER_MAX; ++result)
    {
        if (!strcmp(name, REGISTER[result]))
        {
            break;
        }
    }
    return result;
}

static inline cgbl_error_e cgbl_debug_register_data(const char *const name, cgbl_register_t *const data)
{
    cgbl_register_e reg = cgbl_debug_register(name);
    if (reg < CGBL_REGISTER_MAX)
    {
        return cgbl_processor_register_read(reg, data);
    }
    return CGBL_FAILURE;
}

static cgbl_error_e cgbl_debug_command_cartridge(const char **const arguments, uint8_t length)
{
    if (debug.rom->data)
    {
        uint8_t value = 0;
        char title[12] = {};
        const char *name = NULL;
        for (uint32_t index = 0; index < (sizeof (title) - 1); ++index)
        {
            title[index] = cgbl_bus_read(CGBL_CARTRIDGE_HEADER_TITLE_BEGIN + index);
            if (!title[index])
            {
                break;
            }
        }
        CGBL_TRACE_INFORMATION("Title:    %s\n", strlen(title) ? title : "UNDEFINED");
        value = cgbl_bus_read(CGBL_CARTRIDGE_HEADER_ROM);
        CGBL_TRACE_INFORMATION("Rom:      %02X (%u banks, %u bytes)\n", value, (value < CGBL_LENGTH(ROM)) ? ROM[value] : ROM[0],
            ((value < CGBL_LENGTH(ROM)) ? ROM[value] : ROM[0]) * CGBL_CARTRIDGE_ROM_WIDTH);
        value = cgbl_bus_read(CGBL_CARTRIDGE_HEADER_RAM);
        CGBL_TRACE_INFORMATION("Ram:      %02X (%u banks, %u bytes)\n", value, (value < CGBL_LENGTH(RAM)) ? RAM[value] : RAM[0],
            ((value < CGBL_LENGTH(RAM)) ? RAM[value] : RAM[0]) * CGBL_CARTRIDGE_RAM_WIDTH);
        name = NULL;
        value = cgbl_bus_read(CGBL_CARTRIDGE_HEADER_MODE);
        for (uint32_t index = 0; index < CGBL_LENGTH(MODE); ++index)
        {
            if (MODE[index].type == value)
            {
                name = MODE[index].name;
                break;
            }
        }
        CGBL_TRACE_INFORMATION("Mode:     %02X (%s)\n", value, name && strlen(name) ? name : "???");
        name = NULL;
        value = cgbl_bus_read(CGBL_CARTRIDGE_HEADER_MAPPER);
        for (uint32_t index = 0; index < CGBL_LENGTH(MAPPER); ++index)
        {
            if (MAPPER[index].type == value)
            {
                name = MAPPER[index].name;
                break;
            }
        }
        CGBL_TRACE_INFORMATION("Mapper:   %02X (%s)\n", value, name && strlen(name) ? name : "????");
        CGBL_TRACE_INFORMATION("Checksum: %02X\n", cgbl_bus_read(CGBL_CARTRIDGE_HEADER_CHECKSUM));
    }
    if (debug.ram->data)
    {
        CGBL_TRACE_INFORMATION("Clock:    %u:%u:%u:%u\n", ((cgbl_cartridge_clock_read(CGBL_CLOCK_DAY_HIGH) << 8) | cgbl_cartridge_clock_read(CGBL_CLOCK_DAY_LOW)) & 511,
            cgbl_cartridge_clock_read(CGBL_CLOCK_HOUR), cgbl_cartridge_clock_read(CGBL_CLOCK_MINUTE), cgbl_cartridge_clock_read(CGBL_CLOCK_SECOND));
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_clock_latch(const char **const arguments, uint8_t length)
{
    cgbl_cartridge_clock_latch();
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_clock_read(const char **const arguments, uint8_t length)
{
    cgbl_clock_e clock = cgbl_debug_clock(arguments[1]);
    if (clock >= CGBL_CLOCK_MAX)
    {
        CGBL_TRACE_ERROR("Unsupported clock: \"%s\"\n", arguments[1]);
        return CGBL_FAILURE;
    }
    CGBL_TRACE_INFORMATION("%02X (%u)", cgbl_cartridge_clock_read(clock));
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_clock_write(const char **const arguments, uint8_t length)
{
    cgbl_clock_e clock = cgbl_debug_clock(arguments[1]);
    if (clock >= CGBL_CLOCK_MAX)
    {
        CGBL_TRACE_ERROR("Unsupported clock: \"%s\"\n", arguments[1]);
        return CGBL_FAILURE;
    }
    cgbl_cartridge_clock_write(clock, strtol(arguments[2], NULL, 16));
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_disassemble(const char **const arguments, uint8_t length)
{
    uint16_t address = 0;
    uint32_t offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    if (length == OPTION[CGBL_COMMAND_DISASSEMBLE].max)
    {
        if ((offset = strtol(arguments[length - 1], NULL, 16)) > 0x100)
        {
            CGBL_TRACE_ERROR("Offset too large: %08X\n", offset);
            return CGBL_FAILURE;
        }
    }
    cgbl_debug_disassemble(address, offset);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_exit(const char **const arguments, uint8_t length)
{
    CGBL_TRACE_INFORMATION("Exiting\n");
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_help(const char **const arguments, uint8_t length)
{
    CGBL_TRACE_INFORMATION("Options:\n");
    for (cgbl_command_e command = 0; command < CGBL_COMMAND_MAX; ++command)
    {
        uint32_t offset = 0;
        char buffer[32] = {};
        snprintf(buffer, sizeof (buffer), "   %s", OPTION[command].name);
        for (offset = strlen(buffer); offset < 10; ++offset)
        {
            buffer[offset] = (offset == (sizeof (buffer) - 1)) ? '\0' : ' ';
        }
        snprintf(buffer + strlen(buffer), sizeof (buffer) - strlen(buffer), "%s", OPTION[command].usage);
        for (offset = strlen(buffer); offset < sizeof (buffer); ++offset)
        {
            buffer[offset] = (offset == (sizeof (buffer) - 1)) ? '\0' : ' ';
        }
        CGBL_TRACE_INFORMATION("%s%s\n", buffer, OPTION[command].description);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_interrupt(const char **const arguments, uint8_t length)
{
    cgbl_interrupt_e interrupt = cgbl_debug_interrupt(arguments[1]);
    if (interrupt >= CGBL_INTERRUPT_MAX)
    {
        CGBL_TRACE_ERROR("Unsupported interrupt: \"%s\"\n", arguments[1]);
        return CGBL_FAILURE;
    }
    cgbl_processor_interrupt(interrupt);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_memory_read(const char **const arguments, uint8_t length)
{
    uint16_t address = 0;
    uint32_t offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    if (length == OPTION[CGBL_COMMAND_MEMORY_READ].max)
    {
        if ((offset = strtol(arguments[length - 1], NULL, 16)) > 0xFFFF)
        {
            CGBL_TRACE_ERROR("Offset too large: %08X\n", offset);
            return CGBL_FAILURE;
        }
    }
    cgbl_debug_memory(address, offset);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_memory_write(const char **const arguments, uint8_t length)
{
    uint16_t address = 0;
    uint32_t offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    data.low = strtol(arguments[2], NULL, 16);
    if (length == OPTION[CGBL_COMMAND_MEMORY_WRITE].max)
    {
        if ((offset = strtol(arguments[length - 1], NULL, 16)) > 0xFFFF)
        {
            CGBL_TRACE_ERROR("Offset too large: %08X\n", offset);
            return CGBL_FAILURE;
        }
    }
    for (uint32_t index = address; index < address + offset; ++index)
    {
        cgbl_bus_write(index, data.low);
        if (cgbl_bus_read(index) != data.low)
        {
            CGBL_TRACE_ERROR("Failed to write memory: [%04X] %02X\n", index, data.low);
            return CGBL_FAILURE;
        }
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
        cgbl_register_t data = {};
        if ((result = cgbl_processor_register_read(reg[index], &data)) != CGBL_SUCCESS)
        {
            break;
        }
        switch (reg[index])
        {
            case CGBL_REGISTER_AF:
                CGBL_TRACE_INFORMATION("AF: %04X (A: %02X, F: %02X) [%c%c%c%c]\n", data.word, data.high, data.low,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
                CGBL_TRACE_INFORMATION("BC: %04X (B: %02X, C: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_DE:
                CGBL_TRACE_INFORMATION("DE: %04X (D: %02X, E: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_HL:
                CGBL_TRACE_INFORMATION("HL: %04X (H: %02X, L: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_PC:
                CGBL_TRACE_INFORMATION("PC: %04X\n", data.word);
                break;
            case CGBL_REGISTER_SP:
                CGBL_TRACE_INFORMATION("SP: %04X\n", data.word);
                break;
            default:
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_register_read(const char **const arguments, uint8_t length)
{
    cgbl_error_e result =CGBL_SUCCESS;
    cgbl_register_e reg = cgbl_debug_register(arguments[1]);
    cgbl_register_t data = {};
    if ((result = cgbl_processor_register_read(reg, &data)) == CGBL_SUCCESS)
    {
        switch (reg)
        {
            case CGBL_REGISTER_AF:
                CGBL_TRACE_INFORMATION("%04X [%c%c%c%c]\n", data.word,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
            case CGBL_REGISTER_DE:
            case CGBL_REGISTER_HL:
            case CGBL_REGISTER_PC:
            case CGBL_REGISTER_SP:
                CGBL_TRACE_INFORMATION("%04X\n", data.word);
                break;
            case CGBL_REGISTER_F:
                CGBL_TRACE_INFORMATION("%02X [%c%c%c%c]\n", data.low,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            default:
                CGBL_TRACE_INFORMATION("%02X\n", data.low);
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_register_write(const char **const arguments, uint8_t length)
{
    cgbl_register_e reg = cgbl_debug_register(arguments[1]);
    cgbl_register_t data = { .word = strtol(arguments[2], NULL, 16) };
    return cgbl_processor_register_write(reg, &data);
}

static cgbl_error_e cgbl_debug_command_reset(const char **const arguments, uint8_t length)
{
    return cgbl_bus_reset(debug.rom, debug.ram);
}

static cgbl_error_e cgbl_debug_command_run(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_RUN].max)
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
        if ((result = cgbl_bus_run_breakpoint(breakpoint)) != CGBL_SUCCESS)
        {
            if (result != CGBL_QUIT)
            {
                if (result == CGBL_BREAKPOINT)
                {
                    CGBL_TRACE_WARNING("Breakpoint: %04X\n", breakpoint);
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
    cgbl_register_t reg = {};
    if (length == OPTION[CGBL_COMMAND_STEP].max)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
    if (cgbl_processor_register_read(CGBL_REGISTER_PC, &reg) == CGBL_SUCCESS)
    {
        cgbl_debug_disassemble(reg.word, 1);
    }
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
            CGBL_TRACE_WARNING("Breakpoint: %04X\n", breakpoint);
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
    CGBL_TRACE_INFORMATION("%u.%u-%x\n", version->major, version->minor, version->patch);
    return CGBL_SUCCESS;
}

static cgbl_error_e (*const COMMAND[CGBL_COMMAND_MAX])(const char **const arguments, uint8_t length) = {
    cgbl_debug_command_exit,
    cgbl_debug_command_cartridge,
    cgbl_debug_command_clock_latch,
    cgbl_debug_command_clock_read,
    cgbl_debug_command_clock_write,
    cgbl_debug_command_disassemble,
    cgbl_debug_command_help,
    cgbl_debug_command_interrupt,
    cgbl_debug_command_memory_read,
    cgbl_debug_command_memory_write,
    cgbl_debug_command_processor,
    cgbl_debug_command_register_read,
    cgbl_debug_command_register_write,
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

static void cgbl_debug_header(void)
{
    const cgbl_version_t *const version = cgbl_version();
    CGBL_TRACE_INFORMATION("CGBL %u.%u-%x\n", version->major, version->minor, version->patch);
    if (debug.path && strlen(debug.path))
    {
        CGBL_TRACE_INFORMATION("Path: %s\n", debug.path);
    }
}

static const char *cgbl_debug_prompt(void)
{
    cgbl_register_t reg = {};
    memset(debug.prompt, 0, sizeof (debug.prompt));
    if (cgbl_processor_register_read(CGBL_REGISTER_PC, &reg) == CGBL_SUCCESS)
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n%s%04X%s> ",
            TRACE[CGBL_LEVEL_PROMPT].begin, reg.word, TRACE[CGBL_LEVEL_PROMPT].end);
    }
    else
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n%s\?\?\?\?%s> ",
            TRACE[CGBL_LEVEL_PROMPT].begin, TRACE[CGBL_LEVEL_PROMPT].end);
    }
    return debug.prompt;
}

cgbl_error_e cgbl_debug_entry(const char *const path, const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&debug, 0, sizeof (debug));
    debug.path = path;
    debug.ram = ram;
    debug.rom = rom;
    cgbl_debug_header();
    for (;;)
    {
        char *input = NULL;
        cgbl_command_e command = CGBL_COMMAND_MAX;
        rl_attempted_completion_function = cgbl_debug_completion;
        if ((input = readline(cgbl_debug_prompt())) && strlen(input))
        {
            uint8_t length = 0;
            char *argument = NULL;
            const char *arguments[5] = {};
            add_history(input);
            argument = strtok(input, " ");
            while (argument)
            {
                if (length >= CGBL_LENGTH(arguments))
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
                CGBL_TRACE_ERROR("Command unsupported: \'%s\'\n", input);
                CGBL_TRACE_WARNING("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((length < OPTION[command].min) || (length > OPTION[command].max))
            {
                CGBL_TRACE_ERROR("Command usage: %s %s\n", OPTION[command].name, OPTION[command].usage);
                CGBL_TRACE_WARNING("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((result = COMMAND[command](arguments, length)) == CGBL_FAILURE)
            {
                CGBL_TRACE_ERROR("Command failed: \'%s\'\n", input);
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
