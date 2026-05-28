#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addr_mode.h"
#include "ini.h"
#include "utils.h"

const char am_names[NADDRMODES][AM_NAME_MAX_LEN] =
{
    [AM_ABS_INDEXED_INDIR]   = "AM_ABS_INDEXED_INDIR",
    [AM_ABS_INDIR]           = "AM_ABS_INDIR",
    [AM_ABS_LONG]            = "AM_ABS_LONG",
    [AM_ABS_LONG_IDX]        = "AM_ABS_LONG_IDX",
    [AM_ABS_X_IDX]           = "AM_ABS_X_IDX",
    [AM_ABS_Y_IDX]           = "AM_ABS_Y_IDX",
    [AM_ABSOLUTE]            = "AM_ABSOLUTE",
    [AM_ACCUMULATOR]         = "AM_ACCUMULATOR",
    [AM_BLOCK_MOVE]          = "AM_BLOCK_MOVE",
    [AM_DIR_IDX_INDIR]       = "AM_DIR_IDX_INDIR",
    [AM_DIR_INDIR]           = "AM_DIR_INDIR",
    [AM_DIR_INDIR_IDX]       = "AM_DIR_INDIR_IDX",
    [AM_DIR_INDIR_LONG]      = "AM_DIR_INDIR_LONG",
    [AM_DIR_INDIR_LONG_IDX]  = "AM_DIR_INDIR_LONG_IDX",
    [AM_DIR_X_IDX]           = "AM_DIR_X_IDX",
    [AM_DIR_Y_IDX]           = "AM_DIR_Y_IDX",
    [AM_DIRECT]              = "AM_DIRECT",
    [AM_IMMEDIATE]           = "AM_IMMEDIATE",
    [AM_IMPLIED]             = "AM_IMPLIED",
    [AM_PC_REL]              = "AM_PC_REL",
    [AM_PC_REL_LONG]         = "AM_PC_REL_LONG",
    [AM_STACK]               = "AM_STACK",
    [AM_STACK_REL]           = "AM_STACK_REL",
    [AM_STACK_REL_INDIR_IDX] = "AM_STACK_REL_INDIR_IDX"
};

const char am_syms[NADDRMODES][AM_SYM_MAX_LEN] =
{
    [AM_ABS_INDEXED_INDIR]   = "(a,x)",
    [AM_ABS_INDIR]           = "(a)",
    [AM_ABS_LONG]            = "al",
    [AM_ABS_LONG_IDX]        = "al,x",
    [AM_ABS_X_IDX]           = "a,x",
    [AM_ABS_Y_IDX]           = "a,y",
    [AM_ABSOLUTE]            = "a",
    [AM_ACCUMULATOR]         = "A",
    [AM_BLOCK_MOVE]          = "xya",
    [AM_DIR_IDX_INDIR]       = "(d,x)",
    [AM_DIR_INDIR]           = "(d)",
    [AM_DIR_INDIR_IDX]       = "(d),y",
    [AM_DIR_INDIR_LONG]      = "[d]",
    [AM_DIR_INDIR_LONG_IDX]  = "[d],y",
    [AM_DIR_X_IDX]           = "d,x",
    [AM_DIR_Y_IDX]           = "d,y",
    [AM_DIRECT]              = "d",
    [AM_IMMEDIATE]           = "#",
    [AM_IMPLIED]             = "i",
    [AM_PC_REL]              = "r",
    [AM_PC_REL_LONG]         = "rl",
    [AM_STACK]               = "s",
    [AM_STACK_REL]           = "d,s",
    [AM_STACK_REL_INDIR_IDX] = "(d,s),y"
};

const uint8_t am_noperands[NADDRMODES] =
{
    [AM_ABS_INDEXED_INDIR]   = UINT8_C(2),
    [AM_ABS_INDIR]           = UINT8_C(2),
    [AM_ABS_LONG]            = UINT8_C(3),
    [AM_ABS_LONG_IDX]        = UINT8_C(3),
    [AM_ABS_X_IDX]           = UINT8_C(2),
    [AM_ABS_Y_IDX]           = UINT8_C(2),
    [AM_ABSOLUTE]            = UINT8_C(2),
    [AM_ACCUMULATOR]         = UINT8_C(0),
    [AM_BLOCK_MOVE]          = UINT8_C(2),
    [AM_DIR_IDX_INDIR]       = UINT8_C(1),
    [AM_DIR_INDIR]           = UINT8_C(1),
    [AM_DIR_INDIR_IDX]       = UINT8_C(1),
    [AM_DIR_INDIR_LONG]      = UINT8_C(1),
    [AM_DIR_INDIR_LONG_IDX]  = UINT8_C(1),
    [AM_DIR_X_IDX]           = UINT8_C(1),
    [AM_DIR_Y_IDX]           = UINT8_C(1),
    [AM_DIRECT]              = UINT8_C(1),
    [AM_IMMEDIATE]           = UINT8_C(1),
    [AM_IMPLIED]             = UINT8_C(0),
    [AM_PC_REL]              = UINT8_C(1),
    [AM_PC_REL_LONG]         = UINT8_C(2),
    [AM_STACK]               = UINT8_C(0),
    [AM_STACK_REL]           = UINT8_C(1),
    [AM_STACK_REL_INDIR_IDX] = UINT8_C(1)
};

addr_mode_t
addr_mode_from_string (const char s[static 1])
{
    const size_t slen = strlen (s);
    size_t i;

    for (i = 0; i < NADDRMODES; i++)
    {
        if (strncmp (am_names[i], s, slen) == 0)
        {
            break;
        }
    }

    return i;
}

