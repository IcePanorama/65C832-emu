#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "opcode.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

static const char *input = "blink.bin";

//static uint8_t get_pc_inc (const opcode_t o[static 1]);

int
main (void)
{
    FILE *fptr = fopen(input, "rb");
    if (!fptr)
    {
        fprintf (stderr, "Failed to open %s\n", input);
        return EXIT_FAILURE;
    }

    uint8_t file[INPUT_FILE_SIZE_B];
    size_t bytes_read = fread (file, sizeof (uint8_t), INPUT_FILE_SIZE_B, fptr);
    fclose (fptr);

    if (bytes_read != INPUT_FILE_SIZE_B)
    {
        fprintf (stderr, "File read error\n");
        return EXIT_FAILURE;
    }

    if (op_init() != 0)
    {
        fprintf (stderr, "Failed to initialize instruction data\n");
        return EXIT_FAILURE;
    }

    uint16_t pc = 0;
    while (pc < INPUT_FILE_SIZE_B)
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
