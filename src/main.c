#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "addr_mode.h"
#include "opcode.h"
#include "opmatrix.h"
#include "utils.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

static opcode_t *fetch_instruction (uint8_t *file, const uint16_t curr_pc, uint16_t new_pc[restrict static 1]);
//static uint8_t get_pc_inc (const opcode_t o[restrict static 1]);

int
main (int argc, char **argv)
{
    uint8_t *file;
    size_t fsize;

    if (argc < 2)
    {
        fprintf (stderr, "Improper usage error: no filename provided.\n");
        return EXIT_FAILURE;
    }

    if (u_load_bin_file (argv[1], &file, &fsize) != 0)
    {
        return EXIT_FAILURE;
    }

    uint16_t pc = 0;
    while (pc < fsize)
    {
        opcode_t *op = NULL;
        uint16_t new_pc;
        op = fetch_instruction (file, pc, &new_pc);
        op_print (op);

        if (strlen (op->code) == 0)
        {
            break;
        }

        (void)new_pc; // fixme: tmp

        uint8_t noperands = op_get_noperands (op);
        pc++;

        for (size_t i = 0; i < noperands; i++, pc++)
        {
            printf ("Skipped byte: 0x%02"PRIX8"\n", file[pc]);
        }
    }

    free (file);
    return EXIT_SUCCESS;
}

/** Fetches the current opcode and increments `pc`. */
opcode_t *
fetch_instruction (
    uint8_t *file,
    const uint16_t curr_pc,
    uint16_t new_pc[restrict static 1]
)
{
    const uint8_t curr_byte = file[curr_pc];
    //printf ("Curr byte: %d\n", curr_byte);
    opcode_t *out = &opcode_matrix[curr_byte];

    return out;
    (void)new_pc;
}
