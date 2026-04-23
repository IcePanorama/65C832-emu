#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "instruction.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

static const char *input = "blink.bin";

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

    if (inst_init() != 0)
    {
        fprintf (stderr, "Failed to initialize instruction data\n");
        return EXIT_FAILURE;
    }

    uint16_t pc = 0;
    while (pc < INPUT_FILE_SIZE_B)
    {
        if ((file[pc] != 0xFF) && (opcode_matrix[file[pc]].opcode == 0xFF))
        {
            printf ("Unrecognized opcode: 0x%02"PRIX8"\n", file[pc]);
            break;
        }

        inst_print(&opcode_matrix[file[pc]]);
        pc += opcode_matrix[file[pc]].base_nbytes;
    }

    return EXIT_SUCCESS;
}
