#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "addr_mode.h"
#include "opcode.h"
#include "utils.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

#if 0
static opcode_t *fetch_instruction (uint8_t *file, const size_t fsize, uint16_t pc[restrict static 1]);
static uint8_t get_pc_inc (const opcode_t o[static 1]);
#endif /* 0 */

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

    if (am_init () != 0)
    {
        fprintf (stderr, "Failed to initialize address mode data\n");
        free (file);
        return EXIT_FAILURE;
    }


    if (op_init() != 0)
    {
        fprintf (stderr, "Failed to initialize instruction data\n");
        free (file);
        return EXIT_FAILURE;
    }

    for (size_t x = 0; x < NOPCODES; x++)
    {
        opcode_t curr = opcode_matrix[x];
        if (curr.reserved)
        {
            continue;
        }
        else if (curr.operand_sz != OPS_FIXED)
        {
            op_print (&curr);
        }
    }

    uint16_t pc = 0;
    while (pc < fsize)
    {
        break;
        opcode_t *curr = &opcode_matrix[file[pc]];
        if ((file[pc] != 0xFF) && (curr->opcode == 0xFF))
        {
            printf ("Unrecognized opcode: 0x%02"PRIX8"\n", file[pc]);
            break;
        }

        //printf ("PC: 0x%04"PRIX16"\n", pc);
        op_print(curr);

#if 0
        uint8_t noperands = op_get_noperands (curr);
        pc++;

        for (size_t i = 0; i < noperands; i++, pc++)
        {
            printf ("Skipped byte: 0x%02"PRIX8"\n", file[pc]);
        }
#endif /* 0 */
    }

    free (file);
    return EXIT_SUCCESS;
}

#if 0
/** Fetches the current opcode and increments `pc`. */
opcode_t *
fetch_instruction (
    uint8_t *file, const size_t fsize, uint16_t pc[restrict static 1]
)
{
    const uint8_t curr_byte = file[*pc];
    const opcode_t *out = &opcode_matrix[curr_byte];
    const uint8_t pc_inc = get_pc_inc (out);
}

// FIXME: LO implementing this!
uint8_t
get_pc_inc (const opcode_t o[static 1])
{
    return 1 + op_get_noperands (curr);
}
#endif /* 0 */
