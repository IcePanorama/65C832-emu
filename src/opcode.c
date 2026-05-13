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

#if 0
static const char *operand_sz_strs[NOPERANDSIZES] =
{
    [OPS_FIXED] = "OPS_FIXED",
    [OPS_A]     = "OPS_A",
    [OPS_XY]    = "OPS_XY",
};
#endif /* 0 */
static const char *inst_fname = "opcodes.ini";
static const char *df_toks[NOCDFTYPES] =
{
    [OCDFT_MNEMONIC]   = "mnemonic",
    [OCDFT_ADDR_MODE]  = "addr_mode",
    [OCDFT_OPERAND_SZ] = "operand_sz",
    [OCDFT_NCYCLES]    = "ncycles",
};

static int process_line (char line[restrict static 1]);
static int proc_new_op (char in[restrict static 1], uint8_t opcode[restrict static 1]);
static int str_to_u8 (char s[restrict static 1], uint8_t u8[restrict static 1]);
static void print (opcode_t o[restrict static 1]);
static int proc_df (char in[restrict static 1], opcode_t o[restrict static 1]);
static oc_df_type_t get_df_type (char in[restrict static 1]);

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
            print (&curr);
            assert (false); // FIXME: todo
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
print (opcode_t o[restrict static 1])
{
    printf (
        "%s %s { opcode = 0x%02"PRIX8", operand_sz = %s"
        ", ncycles = 0x%"PRIX8" }\n",
        "", //o->mnemonic,
        "", //o->addr_mode.symbol,
        o->opcode,
        "", //operand_sz_strs[o->operand_sz],
        0 //o->ncycles
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
                break;
            case OCDFT_ADDR_MODE:
                break;
            case OCDFT_OPERAND_SZ:
                break;
            case OCDFT_NCYCLES:
                break;
            default:
                assert (false); // should be impossible to reach here.
        }
    }

    return ret;
    (void)o;
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

