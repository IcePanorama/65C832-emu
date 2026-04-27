#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "opcode.h"
#include "utils.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

//static uint8_t get_pc_inc (const opcode_t o[static 1]);

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

    if (op_init() != 0)
    {
        fprintf (stderr, "Failed to initialize instruction data\n");
        free (file);
        return EXIT_FAILURE;
    }


    uint16_t pc = 0;
    while (pc < fsize)
    {
        opcode_t *curr = &opcode_matrix[file[pc]];
        if ((file[pc] != 0xFF) && (curr->opcode == 0xFF))
        {
            printf ("Unrecognized opcode: 0x%02"PRIX8"\n", file[pc]);
            break;
        }

        //printf ("PC: 0x%04"PRIX16"\n", pc);
        op_print(curr);

        uint8_t noperands = op_get_noperands (curr);
        pc++;

        for (size_t i = 0; i < noperands; i++, pc++)
        {
            printf ("Skipped byte: 0x%02"PRIX8"\n", file[pc]);
        }
    }

    free (file);
    return EXIT_SUCCESS;
}

#if 0 // FIXME: This is needed for execution, not decoding.
uint8_t
get_pc_inc (const opcode_t o[static 1])
{
    if (o->opcode == 0x00) return 2; // BRK
    if (o->opcode == 0x02) return 2; // COP (65816)

    return 1 + op_get_noperands (curr);
}
#endif /* 0 */
