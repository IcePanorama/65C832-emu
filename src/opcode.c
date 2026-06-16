#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcode.h"
#include "opmatrix.h"
#include "ini.h"
#include "utils.h"

static const char *sz_toks[NOPERANDSIZES] =
{
    [OPS_FIXED] = "OPS_FIXED",
    [OPS_A]     = "OPS_A",
    [OPS_XY]    = "OPS_XY",
};
static const size_t largest_sz_tok_len = 9;  // = strlen (sz_toks[OPS_FIXED]);

void
op_print (const opcode_t o[restrict static 1])
{
    fprintf (stdout,
        "%s %-*s { operand_sz = %*s, ncycles = %3"PRIu8" }\n",
        o->mnemonic,
        AM_SYM_MAX_LEN,
        am_syms[o->addr_mode],
        (int)largest_sz_tok_len,
        sz_toks[o->operand_sz],
        o->ncycles
    );
}

// FIXME: Hardcoded values
uint8_t
op_get_noperands (const opcode_t o[static 1])
{
    uint8_t mod = 0;

    switch (o->operand_sz)
    {
        case OPS_FIXED:
            break;
        case OPS_A:
            mod = 0; // FIXME: Assuming 8-bit accumulator
            break;
        case OPS_XY:
            mod = 3; // FIXME: Assuming 32-bit x/y registers
            break;
        case NOPERANDSIZES:
        default:
            assert (false); // should never reach here.
    }

    return am_noperands[o->addr_mode] + mod;
}

