#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcode.h"
#include "ini.h"
#include "utils.h"

typedef enum oc_df_type_e
{
    OCDFT_MNEMONIC,
    OCDFT_ADDR_MODE,
    OCDFT_OPERAND_SZ,
    OCDFT_NCYCLES,
    NOCDFTYPES
} oc_df_type_t;

opcode_t opcode_matrix[NOPCODES];

static const char *inst_fname = "opcodes.ini";
static const char *df_toks[NOCDFTYPES] =
{
    [OCDFT_MNEMONIC]   = "mnemonic",
    [OCDFT_ADDR_MODE]  = "addr_mode",
    [OCDFT_OPERAND_SZ] = "operand_sz",
    [OCDFT_NCYCLES]    = "ncycles",
};
static const char *sz_toks[NOPERANDSIZES] =
{
    [OPS_FIXED] = "OPS_FIXED",
    [OPS_A]     = "OPS_A",
    [OPS_XY]    = "OPS_XY",
};
static const size_t largest_sz_tok_len = 9;  // = strlen (sz_toks[OPS_FIXED]);

static int process_line (char line[restrict static 1]);
static int proc_new_op (char in[restrict static 1], uint8_t opcode[restrict static 1]);
static int str_to_u8 (char s[restrict static 1], uint8_t u8[restrict static 1]);
static int proc_df (char in[restrict static 1], opcode_t o[restrict static 1]);
static oc_df_type_t get_df_type (char in[restrict static 1]);
static int proc_mnemonic_df (const char value[restrict static 1], char mnemonic[restrict static OP_MNEMONIC_LEN]);
static int proc_addr_mode_df (const char value[restrict static 1], addr_mode_t *am[restrict static 1]);
static int proc_operand_sz_df (const char value[restrict static 1], operand_sz_t sz[restrict static 1]);
static int proc_ncycles_df (char value[restrict static 1], uint8_t ncycles[restrict static 1]);

int
op_init (void)
{
    if (ini_process_file (inst_fname, &process_line) != 0)
    {
        return -1;
    }

    return 0;
}

int
process_line (char line[restrict static 1])
{
    static opcode_t curr = { 0 };
    static bool is_new_op = true;
    int ret = 0;
    char cpy[256];
    uint8_t op;

    strncpy (cpy, line, strlen (line) + 1);

    if (is_new_op)
    {
        if (proc_new_op (cpy, &op) != 0)
        {
            fprintf (stderr, "Malformed input: %s", line);
            ret = -1;
        }
        else
        {
            curr.opcode = op;
            is_new_op = true;
            is_new_op = false;
        }
    }
    else
    {
        if ((*cpy == '\n') || (*cpy == '\r'))
        {
            opcode_matrix[curr.opcode] = curr;
            is_new_op = true;
        }
        else if (proc_df (cpy, &curr) != 0)
        {
            ret = -1;
        }
    }

    return ret;
    (void)curr;
}

int
proc_new_op (
    char in[restrict static 1],
    uint8_t opcode[restrict static 1]
)
{
    int ret = 0;
    uint8_t op = 0;
    char out[INI_MAX_STR_LEN];

    ret = ini_get_section_title (in, out);
    if (ret == 0)
    {
        char *tmp = out;
        size_t op_size;

        if (
            (strncmp (tmp, "0x", 2) == 0)
            || (strncmp (tmp, "0X", 2) == 0)
        )
        {
            tmp += 2;
        }

        op_size = strlen (tmp);

        if (op_size > 4)
        {
            fprintf (stderr,
                "Given op code is too long to fit in a "
                "uint8_t: %s\n",
                out
            );

            ret = -1;
        }
        else if (str_to_u8 (tmp, &op) != 0)
        {
            ret = -1;
        }
    }

    if (ret == 0)
    {
        *opcode = op;
    }

    return 0;
}


int
str_to_u8 (
    char s[restrict static 1], uint8_t u8[restrict static 1]
)
{
    int ret = 0;
    char *tmp = s;
    uint8_t out = 0;

    while (*tmp != '\0')
    {
        out <<= 4;

        if (('0' <= *tmp) && (*tmp <= '9'))
        {
            out |= *tmp - '0';
        }
        else if (('a' <= *tmp) && (*tmp <= 'f'))
        {
            out |= *tmp - 'a';
        }
        else if (('A' <= *tmp) && (*tmp <= 'F'))
        {
            out |= *tmp - 'A';
        }
        else
        {
            fprintf (
                stderr,
                "Invalid hex string: %s\n",
                tmp
            );

            ret = -1;
            break;
        }

        tmp++;
    }

    if (ret == 0)
    {
        *u8 = out;
    }

    return ret;
}

void
op_print (const opcode_t o[restrict static 1])
{
    fprintf (stdout,
        "%s %-*s { opcode = 0x%02"PRIX8", operand_sz = %*s"
        ", ncycles = %3"PRId8" }\n",
        o->mnemonic,
        AM_SYM_MAX_LEN,
        o->addr_mode->symbol,
        o->opcode,
        (int)largest_sz_tok_len,
        sz_toks[o->operand_sz],
        o->ncycles
    );
}

int
proc_df (
    char in[restrict static 1], opcode_t o[restrict static 1]
)
{
    const oc_df_type_t type = get_df_type (in);
    int ret = 0;
    char value[INI_MAX_STR_LEN] = { 0 };

    if (ini_get_value (in, value) != 0)
    {
        fprintf (
            stderr,
            "Failed to read value in line: %s",
            in
        );
        ret = -1;
    }
    else
    {
        switch (type)
        {
            case OCDFT_MNEMONIC:
                if (proc_mnemonic_df (value, o->mnemonic) != 0)
                {
                    ret = -1;
                }

                break;
            case OCDFT_ADDR_MODE:
                if (proc_addr_mode_df (value, &o->addr_mode) != 0)
                {
                    ret = -1;
                }

                break;
            case OCDFT_OPERAND_SZ:
                if (proc_operand_sz_df (value, &o->operand_sz) != 0)
                {
                    ret = -1;
                }

                break;
            case OCDFT_NCYCLES:
                if (proc_ncycles_df (value, &o->ncycles) != 0)
                {
                    ret = -1;
                }

                break;
            default:
                assert (false); // should be impossible to reach here.
        }
    }

    return ret;
}

oc_df_type_t
get_df_type (char in[restrict static 1])
{
    oc_df_type_t out = NOCDFTYPES;

    for (size_t i = 0; i < NOCDFTYPES; i++)
    {
        const char *curr_tok = df_toks[i];
        if (strncmp (curr_tok, in, strlen (curr_tok)) == 0)
        {
            out = (oc_df_type_t)i;
            break;
        }
    }

    return out;
}

int
proc_mnemonic_df (
    const char value[restrict static 1],
    char mnemonic[restrict static OP_MNEMONIC_LEN]
)
{
    const size_t val_len = strlen (value);
    int ret = 0;

    if (val_len >= OP_MNEMONIC_LEN)
    {
        fprintf (
            stderr,
            "Given mnemonic too long: %zu > %d\n",
            val_len,
            OP_MNEMONIC_LEN
        );
        ret = -1;
    }
    else
    {
        strncpy (mnemonic, value, val_len);
    }

    return ret;
}

int
proc_addr_mode_df (
    const char value[restrict static 1],
    addr_mode_t *am[restrict static 1]
)
{
    addr_mode_t *mode = addr_mode_from_string (value);
    int ret = 0;

    if (mode == NULL)
    {
        fprintf (stderr, "Invalid addr mode: %s\n", value);
        ret = -1;
    }
    else
    {
        *am = mode;
    }

    return ret;
}

int
proc_operand_sz_df (
    const char value[restrict static 1], operand_sz_t sz[restrict static 1]
)
{
    const size_t vlen = strlen (value);
    int ret = 0;
    operand_sz_t out = NOPERANDSIZES;
    size_t i;

    for (i = 0; i < NOPERANDSIZES; i++)
    {
        const char *str = sz_toks[i];
        if (strncmp (value, str, vlen) == 0)
        {
            out = (operand_sz_t)i;
            break;
        }
    }

    if (out == NOPERANDSIZES)
    {
        fprintf (stderr, "Invalid operand size: %s\n", value);
        ret = -1;
    }
    else
    {
        *sz = out;
    }

    return ret;
}

int
proc_ncycles_df (
    char value[restrict static 1], uint8_t ncycles[restrict static 1]
)
{
    uintmax_t out = 0;
    int ret = 0;
    char *c;

    for (c = value; *c != '\0'; c++)
    {
        out *= 10;
        if (('0' <= *c) || (*c <= '9'))
        {
            out += *c - '0';
        }
        else
        {
            fprintf (
                stderr,
                "Invalid ncycles value: got %s, natural number expected\n",
                value
            );
            ret = -1;
            break;
        }
    }

    if (ret == 0)
    {
        if (out > UINT8_MAX)
        {
            fprintf (
                stderr,
                "Invalid ncycles value: %"PRIdMAX" > %"PRId8"\n",
                out,
                UINT8_MAX
            );
            ret = -1;
        }
        else
        {
            *ncycles = (uint8_t)out;
        }
    }

    return ret;
}

