#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "instruction.h"

instruction_t opcode_matrix[NINSTRUCTIONS];

static const char *inst_fname = "opcodes.ini";

static int proc_new_opcode (char in[static 1], char *mnemonic[static 1]);
static int proc_datafield (char in[static 1], instruction_t i[static 1]);

int
inst_init (void)
{
    enum init_state_e { NEW_OPC, OPC_PARSE, OPC_DONE, NSTATES } state = NEW_OPC;
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
                    goto loop_end;
                }

                strncpy (curr.mnemonic, mnemonic, MNEMONIC_LEN);
                ptr += strlen (mnemonic) + 1;

                state = (state + 1) % NSTATES;
                break;
            case OPC_PARSE:
                // FIXME: Probably not the best way of doing this.
                if (strlen(cpy) == 1)
                {
                    state = (state + 1) % NSTATES;
                }
                else if (proc_datafield (cpy, &curr) != 0)
                {
                    // TODO
                    goto loop_end;
                }

                break;
            case OPC_DONE:
                //FIXME: tmp
                goto loop_done;
            default:
                break;
        }

        if (state == NSTATES)
        {
            break;
        }
    }

loop_done:
    inst_print (&curr);

loop_end:
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
    enum datafields_e
    {
        OPCODE,
        ADDR_MODE,
        BASE_NBYTES,
        BASE_NCYCLES
    } type;

    if (strncmp (in, "opcode", 6) == 0)
    {
        type = OPCODE;
    }
    else if (strncmp (in, "addr_mode", 9) == 0)
    {
        type = ADDR_MODE;
    }
    else if (strncmp (in, "base_nbytes", 11) == 0)
    {
        type = BASE_NBYTES;
    }
    else if (strncmp (in, "base_ncycles", 12) == 0)
    {
        type = BASE_NCYCLES;
    }
    else
    {
        fprintf (stderr, "Unrecognized data field in line: %s", in);
        return -1;
    }

    char *data = strchr (in, '=');
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

    printf ("Data: %s", data);

    switch (type)
    {
        case OPCODE:
            if (strncmp (data, "0x", 2) == 0)
            {
                data += 2;
            }

            uint8_t value = 0;
            char *tmp = data;
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

                tmp++;
            }

            i->opcode = value;
            break;
        case ADDR_MODE:
            // FIXME: LO HERE!
            break;
        default:
            break;
    }

    return 0;
}

void
inst_print (instruction_t i[static 1])
{
    const char *addr_mode = addressing_mode_sym[i->addr_mode];

    printf (
        "0x%"PRIX8" - %s [%s] nbytes: %"PRIu8", ncycles: %"PRIu8"\n",
        i->opcode,
        i->mnemonic,
        addr_mode,
        i->base_nbytes,
        i->base_ncycles
    );
}
