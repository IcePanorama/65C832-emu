#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"

typedef enum df_type_e
{
    DF_OPCODE, DF_ADDR_MODE, DF_BASE_NBYTES, DF_BASE_NCYCLES, NDFTYPES
} df_type_t;

instruction_t opcode_matrix[NINSTRUCTIONS];

static const char *inst_fname = "opcodes.ini";

static int proc_new_opcode (char in[static 1], char *mnemonic[static 1]);
static int proc_datafield (char in[static 1], instruction_t i[static 1]);
static df_type_t proc_df_get_type (char in[static 1]);
static int proc_df_opcode (char in[static 1], instruction_t i[static 1]);
static int proc_df_addr_mode (char in[static 1], instruction_t i[static 1]);
static int proc_df_read_u8 (char in[static 1], uint8_t out[static 1]);
static int proc_df_base_nbytes (char in[static 1], instruction_t i[static 1]);
static int proc_df_base_ncycles (char in[static 1], instruction_t i[static 1]);

int
inst_init (void)
{
    enum init_state_e { NEW_OPC, OPC_PARSE, NSTATES } state = NEW_OPC;
    FILE *fptr = NULL;
    int ret = 0;

    memset (opcode_matrix, 0xFF, sizeof (instruction_t) * NINSTRUCTIONS);

    fptr = fopen (inst_fname, "r");
    if (fptr == NULL)
    {
        fprintf (stderr, "Failed to open instruction data file: %s\n", inst_fname);
        return -1;
    }

    char line[256] = { 0 };
    char cpy[256];
    instruction_t curr = { 0 };
    bool should_exit = false;
    while (fgets (line, sizeof (line), fptr))
    {
        char *mnemonic;
        char *ptr;

        strncpy (cpy, line, sizeof (line));
        ptr = cpy;

        switch (state)
        {
            case NEW_OPC:
               if (proc_new_opcode (cpy, &mnemonic) != 0)
                {
                    fprintf (stderr, "Malformed input: %s", line);
                    should_exit = true;
                    break;
                }

                strncpy (curr.mnemonic, mnemonic, MNEMONIC_LEN);
                ptr += strlen (mnemonic) + 1;

                state = (state + 1) % NSTATES;

                break;
            case OPC_PARSE:
                if ((*cpy == '\n') || (*cpy == '\r'))
                {
                    memcpy (
                        &opcode_matrix[curr.opcode],
                        &curr,
                        sizeof (instruction_t)
                    );
                    memset (&curr, 0, sizeof (instruction_t));

                    state = (state + 1) % NSTATES;
                }
                else if (proc_datafield (cpy, &curr) != 0)
                {
                    should_exit = true;
                    break;
                }

                break;
            default:
                assert (false); // should never reach here.
        }

        if (should_exit)
        {
            break;
        }
    }

    fclose (fptr);
    return ret;
}

int
proc_new_opcode (char in[static 1], char *mnemonic[static 1])
{
    char *tok;
    char *ptr;
    int ret = 0;

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
        *mnemonic = ptr;
    }

    return ret;
}

int
proc_datafield (char in[static 1], instruction_t i[static 1])
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
        case DF_OPCODE:
            if (proc_df_opcode (data, i) != 0)
            {
                ret = -1;
            }

            break;
        case DF_ADDR_MODE:
            if (proc_df_addr_mode (data, i) != 0)
            {
                ret = -1;
            }

            break;
        case DF_BASE_NBYTES:
            if (proc_df_base_nbytes (data, i) != 0)
            {
                ret = -1;
            }

            break;
        case DF_BASE_NCYCLES:
            if (proc_df_base_ncycles (data, i) != 0)
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

    if (strncmp (in, "opcode", 6) == 0)
    {
        out = DF_OPCODE;
    }
    else if (strncmp (in, "addr_mode", 9) == 0)
    {
        out = DF_ADDR_MODE;
    }
    else if (strncmp (in, "base_nbytes", 11) == 0)
    {
        out = DF_BASE_NBYTES;
    }
    else if (strncmp (in, "base_ncycles", 12) == 0)
    {
        out = DF_BASE_NCYCLES;
    }
    else
    {
        fprintf (stderr, "Unrecognized data field: %s", in);
    }

    return out;
}

int
proc_df_opcode (char in[static 1], instruction_t i[static 1])
{
    int ret = 0;
    uint8_t value = 0;
    char *tmp = in;

    if (strncmp (tmp, "0x", 2) == 0)
    {
        tmp += 2;
    }

    while (*tmp != '\0')
    {
        if (('0' <= *tmp) && (*tmp <= '9'))
        {
            value = (value << 4) + (*tmp - '0');
        }
        else if (('a' <= *tmp) && (*tmp <= 'f'))
        {
            value = (value << 4) + (*tmp - 'a' + 10);
        }
        else if (('A' <= *tmp) && (*tmp <= 'F'))
        {
            value = (value << 4) + (*tmp - 'A' + 10);
        }
        else if ((*tmp == '\n') || (*tmp == '\r'))
        {
            break;
        }
        else
        {
            fprintf (stderr, "Malformed input (hex string expected): %s", in);
            ret = -1;
            break;
        }

        tmp++;
    }

    if (ret == 0)
    {
        i->opcode = value;
    }

    return ret;
}

int
proc_df_addr_mode (char in[static 1], instruction_t i[static 1])
{
    const addressing_mode_t mode = addr_mode_from_string (in);
    int ret = 0;

    if (mode == NADDRESSING_MODES)
    {
        fprintf (stderr, "Malformed input (invalid addressing mode): %s", in);
        ret = -1;
    }
    else
    {
        i->addr_mode = mode;
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
proc_df_base_nbytes (char in[static 1], instruction_t i[static 1])
{
    uint8_t nbytes;
    int ret = 0;

    if (proc_df_read_u8 (in, &nbytes) != 0)
    {
        ret = -1;
    }
    else
    {
        i->base_nbytes = nbytes;
    }

    return ret;
}

int
proc_df_base_ncycles (char in[static 1], instruction_t i[static 1])
{
    uint8_t ncycles;
    int ret = 0;

    if (proc_df_read_u8 (in, &ncycles) != 0)
    {
        ret = -1;
    }
    else
    {
        i->base_ncycles = ncycles;
    }

    return ret;
}

void
inst_print (instruction_t i[static 1])
{
    const char *addr_mode = addr_mode_sym[i->addr_mode];

    printf (
        "0x%02"PRIX8" - %s [%s] nbytes: %"PRIu8", ncycles: %"PRIu8"\n",
        i->opcode,
        i->mnemonic,
        addr_mode,
        i->base_nbytes,
        i->base_ncycles
    );
}
