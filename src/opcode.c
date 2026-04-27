#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "opcode.h"

typedef enum df_type_e
{
    DF_MNEMONIC, DF_ADDR_MODE, DF_SIZE, DF_NCYCLES, NDFTYPES
} df_type_t;

typedef enum init_state_e
{
    NEW_OPC, OPC_PARSE, NSTATES
} init_state_t;

opcode_t opcode_matrix[NOPCODES];

// FIXME: tmp, make global/fn instead
// FIXME: We can (and probably should) load these from a config file also
uint8_t base_noperands[NADDRESSING_MODES] =
{
    [AM_ABS_INDEXED_INDIR] = 2,
    [AM_ABS_X_IDX] = 2,
    [AM_ABS_Y_IDX] = 2,
    [AM_ABS_INDIR] = 2,
    [AM_ABS_LONG_IDX] = 3,
    [AM_ABS_LONG] = 3,
    [AM_ABSOLUTE] = 2,
    [AM_ACCUMULATOR] = 0,
    [AM_BLOCK_MOVE] = 2,
    [AM_DIR_IDX_INDIR] = 1,
    [AM_DIR_INDIR_IDX] = 1,
    [AM_DIR_INDIR_LONG_IDX] = 1,
    [AM_DIR_INDIR_LONG] = 1,
    [AM_DIR_INDIR] = 1,
    [AM_DIR_X_IDX] = 1,
    [AM_DIRECT] = 1,
    [AM_IMPLIED] = 0,
    [AM_IMMEDIATE] = 1,
    [AM_PC_REL_LONG] = 2,
    [AM_PC_REL] = 1,
    [AM_STACK_REL] = 1,
    [AM_STACK_REL_INDIR_IDX] = 1,
    [AM_STACK] = 0,
};

static const char *sz_names[NOPERANDSIZES] = {
    [OPS_FIXED] = "OPS_FIXED",
    [OPS_A] = "OPS_A",
    [OPS_XY] = "OPS_XY"
};

static const char *inst_fname = "opcodes.ini";

static int proc_new_opcode (char *in, uint8_t opcode[static 1]);
static int proc_datafield (char in[static 1], opcode_t i[static 1]);
static df_type_t proc_df_get_type (char in[static 1]);
static int proc_df_mnemonic (char in[static 1], opcode_t i[static 1]);
static int proc_df_addr_mode (char in[static 1], opcode_t i[static 1]);
static int proc_df_size (char in[static 1], opcode_t o[static 1]);
static int proc_df_read_u8 (char in[static 1], uint8_t out[static 1]);
static int proc_df_ncycles (char in[static 1], opcode_t i[static 1]);
static int load_file (const char path[static 1], char **fdata[restrict static 1], size_t nlines[restrict static 1]);
static int get_lines (FILE f[static 1], char **fdata[restrict static 1], size_t nlines[restrict static 1]);
static int proc_file (char *fdata[restrict static 1], const size_t nlines);
static int proc_line (char line[static 1], opcode_t curr[static 1], init_state_t state[static 1]);

int
op_init (void)
{
    size_t nlines = 0;
    char **fdata;
    if (load_file (inst_fname, &fdata, &nlines) != 0)
    {
        fprintf (stderr, "Failed to load opcode data from file: %s\n", inst_fname);
        return -1;
    }

    proc_file (fdata, nlines);

    for (size_t i = 0; i < nlines; i++)
    {
        free (fdata[i]);
    }
    free (fdata);
    return 0;
}

int
load_file (
    const char path[static 1],
    char **fdata[restrict static 1],
    size_t nlines[restrict static 1]
)
{
    FILE *fptr = NULL;
    char **out;
    int ret = 0;

    fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf (stderr, "Failed to open file: %s\n", path);
        return -1;
    }

    out = malloc (sizeof (char *));
    if (out == NULL)
    {
        fprintf (stderr, "%s: Out of memory error.\n", __func__);
        ret = -1;
    }
    else if (get_lines (fptr, &out, nlines) != 0)
    {
        size_t i;
        for (i = 0; i < *nlines; i++)
        {
            free (out[i]);
        }

        free (out);
        *nlines = 0;
        ret = -1;
    }
    else
    {
        *fdata = out;
    }

    fclose (fptr);
    return ret;
}

static inline int
resize_fdata (char **fdata[restrict static 1], size_t max_lines[static 1])
{
    const size_t new_max = *max_lines * 2;
    char **tmp = realloc (*fdata, sizeof (char *) * new_max);
    int ret = 0;

    if (tmp == NULL)
    {
        ret = -1;
    }
    else
    {
        *max_lines = new_max;
        *fdata = tmp;
    }

    return ret;
}

int
get_lines (FILE f[static 1], char **fdata[restrict static 1], size_t nlines[restrict static 1])
{
    char line[256] = { 0 };
    size_t ln_cnt, max_lines;
    int ret = 0;

    ln_cnt = 0;
    max_lines = 1;

    while (fgets (line, sizeof (line), f))
    {
        const size_t ln_len = strlen (line) + 1;
        char *cpy = malloc (ln_len);

        if (ln_cnt == max_lines)
        {
            if (resize_fdata (fdata, &max_lines) != 0)
            {
                free (cpy);
                ret = -1;
                break;
            }
        }

        if (cpy == NULL)
        {
            ret = -1;
            break;
        }
        else
        {
            strncpy (cpy, line, ln_len);

            (*fdata)[ln_cnt] = cpy;
            ln_cnt++;
        }
    }

    *nlines = ln_cnt;
    return ret;
}

int
proc_file (char *fdata[restrict static 1], const size_t nlines)
{
    opcode_t curr = { 0 };
    init_state_t state = NEW_OPC;
    size_t i;

    for (i = 0; i < nlines; i++)
    {
        if (proc_line (fdata[i], &curr, &state) != 0)
        {
            break;
        }
    }

    return 0;
}

int proc_line (char line[static 1], opcode_t curr[static 1], init_state_t state[static 1])
{
    char cpy[256];
    int ret = 0;
    uint8_t opcode;

    strncpy (cpy, line, strlen (line) + 1);

    switch (*state)
    {
        case NEW_OPC:
            if (proc_new_opcode (cpy, &opcode) != 0)
            {
                fprintf (stderr, "Malformed input: %s", line);
                ret = -1;
            }
            else
            {
                curr->opcode = opcode;
                *state = (*state + 1) % NSTATES;
            }

            break;
        case OPC_PARSE:
            if ((*cpy == '\n') || (*cpy == '\r'))
            {
                memcpy (&opcode_matrix[curr->opcode], curr, sizeof (opcode_t));
                op_print (curr);
                memset (curr, 0, sizeof (opcode_t));

                *state = (*state + 1) % NSTATES;
            }
            else if (proc_datafield (cpy, curr) != 0)
            {
                ret = -1;
            }

            break;
        default:
            assert (false); // should never reach here.
    }

    return ret;
}
int
proc_new_opcode (char *in, uint8_t opcode[static 1])
{
    char *tok;
    char *ptr;
    int ret = 0;
    uintmax_t value = 0;

    tok = strchr (in, '[');
    if ((tok == NULL) || (*tok == '\0'))
    {
        return -1;
    }

    ptr = tok + 1;
    tok = strchr (in, ']');
    if ((tok == NULL) || (*tok == '\0'))
    {
        ret = -1;
    }
    else
    {
        *tok = '\0';
    }

    if (strncmp (ptr, "0x", 2) == 0)
    {
        ptr += 2;
    }

    tok = ptr;
    while (*tok != '\0')
    {
        if (('0' <= *tok) && (*tok <= '9'))
        {
            value = (value << 4) + (*tok - '0');
        }
        else if (('a' <= *tok) && (*tok <= 'f'))
        {
            value = (value << 4) + (*tok - 'a' + 10);
        }
        else if (('A' <= *tok) && (*tok <= 'F'))
        {
            value = (value << 4) + (*tok - 'A' + 10);
        }
        else if ((*tok == '\n') || (*tok == '\r'))
        {
            break;
        }
        else
        {
            fprintf (stderr, "Malformed input (hex string expected): %s\n", ptr);
            ret = -1;
            break;
        }

        tok++;
    }

    if (value > UINT8_MAX)
    {
        fprintf (stderr, "Malformed opcode (given value too large): %s (255 max)", ptr);
        ret = -1;
    }
    else
    {
        *opcode = (uint8_t)value;
    }

    return ret;
}

int
proc_datafield (char in[static 1], opcode_t o[static 1])
{
    const df_type_t type = proc_df_get_type (in);
    char *data = strchr (in, '=');
    int ret = 0;

    if ((data == NULL) || (*data == '\0'))
    {
        fprintf (stderr, "Malformed line (no equal sign): %s", in);
        return -1;
    }

    do
    {
        data++;
    }
    while (*data == ' ' && *data != '\0');

    if (*data == '\0')
    {
        fprintf (stderr, "Malformed line (no value): %s", in);
        return -1;
    }

    switch (type)
    {
        case DF_MNEMONIC:
            if (proc_df_mnemonic (data, o) != 0)
            {
                ret = -1;
            }

            break;
        case DF_ADDR_MODE:
            if (proc_df_addr_mode (data, o) != 0)
            {
                ret = -1;
            }

            break;
        case DF_SIZE:
            if (proc_df_size (data, o) != 0)
            {
                ret = -1;
            }

            break;
        case DF_NCYCLES:
            if (proc_df_ncycles (data, o) != 0)
            {
                ret = -1;
            }

            break;
        default:
            printf ("Data: %s", data);
            ret = -1;
            break;
    }

    return ret;
}

df_type_t
proc_df_get_type (char in[static 1])
{
    df_type_t out = NDFTYPES;

    if (strncmp (in, "mnemonic", 8) == 0)
    {
        out = DF_MNEMONIC;
    }
    else if (strncmp (in, "addr_mode", 9) == 0)
    {
        out = DF_ADDR_MODE;
    }
    else if (strncmp (in, "operand_sz", 10) == 0)
    {
        out = DF_SIZE;
    }
    else if (strncmp (in, "ncycles", 7) == 0)
    {
        out = DF_NCYCLES;
    }
    else
    {
        fprintf (stderr, "Unrecognized data field: %s", in);
    }

    return out;
}

int
proc_df_mnemonic (char in[static 1], opcode_t o[static 1])
{
    const size_t len = strlen (in);
    char *last_ch = &in[len - 1];

    if ((*last_ch == '\n') || (*last_ch == '\r'))
    {
        *last_ch = '\0';
    }

    strncpy (o->mnemonic, in, len);

    return 0;
}

int
proc_df_addr_mode (char in[static 1], opcode_t o[static 1])
{
    const addr_mode_t mode = addr_mode_from_string (in);
    int ret = 0;

    if (mode == NADDRESSING_MODES)
    {
        fprintf (stderr, "Malformed input (invalid addressing mode): %s", in);
        ret = -1;
    }
    else
    {
        o->addr_mode = mode;
    }

    return ret;
}

int
proc_df_size (char in[static 1], opcode_t o[static 1])
{
    const operand_sz_t sz = op_get_sz_from_str (in);
    int ret = 0;

    if (sz == NOPERANDSIZES)
    {
        fprintf (stderr, "Malformed input (invalid operand size): %s", in);
        ret = -1;
    }
    else
    {
        o->operand_sz = sz;
    }

    return ret;
}

int
proc_df_read_u8 (char in[static 1], uint8_t out[static 1])
{
    char *tmp = in;
    int ret = 0;
    uintmax_t val = 0;

    while (*tmp != '\0')
    {
        if (('0' <= *tmp) && (*tmp <= '9'))
        {
            val *= 10;
            val += *tmp - '0';
        }
        else if ((*tmp == '\n') || (*tmp == '\r'))
        {
            break;
        }
        else
        {
            fprintf (stderr, "Malformed input (integer value expected): %s", in);
            ret = -1;
            break;
        }

        tmp++;
    }

    if (ret == 0)
    {
        if (val > UINT8_MAX)
        {
            fprintf (
                stderr,
                "Malformed input (given value too large): %s (255 max)",
                in
            );
            ret = -1;
        }
        else if (val == 0)
        {
            fprintf (
                stderr,
                "Malformed input: zero value where non-zero value expected"
            );
            ret = -1;
        }
        else
        {
            *out = (uint8_t)val;
        }
    }

    return ret;
}

int
proc_df_ncycles (char in[static 1], opcode_t o[static 1])
{
    uint8_t ncycles;
    int ret = 0;

    if (proc_df_read_u8 (in, &ncycles) != 0)
    {
        ret = -1;
    }
    else
    {
        o->ncycles = ncycles;
    }

    return ret;
}

void
op_print (opcode_t o[static 1])
{
    const char *addr_mode = addr_mode_sym[o->addr_mode];

    printf (
        "0x%02"PRIX8" - %s [%s] ncycles: %"PRIu8"\n", 
        o->opcode, 
        o->mnemonic,
        addr_mode,
        o->ncycles
    );
}

// FIXME: A register can be diff size from X/Y, chage res based on operand sz
uint8_t
op_get_noperands (const opcode_t o[static 1])
{
    uint8_t res;

    switch (o->addr_mode)
    {
        case AM_IMMEDIATE:
            switch (o->operand_sz)
            {
                case OPS_A:
                    res = 1;
                    break;
                case OPS_XY:
                    res = 4;
                    break;
                case OPS_FIXED:
                    // fall through
                default:
                    res = 1;
                    break;
            }

            break;
        default:
            res = base_noperands[o->addr_mode];
            break;
    };

    return res;
}

// FIXME: Could be static?
operand_sz_t
op_get_sz_from_str (const char s[static 1])
{
    operand_sz_t out = NOPERANDSIZES;

    for (size_t i = 0; i < NOPERANDSIZES; i++)
    {
        if (strncmp (s, sz_names[i], strlen (sz_names[i])) == 0)
        {
            out = (operand_sz_t)i;
            break;
        }
    }

    return out;
}

