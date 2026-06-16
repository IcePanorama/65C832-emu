#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "addr_mode.h"
#include "cpu.h"
#include "opcode.h"
#include "opmatrix.h"
#include "utils.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

uint8_t *file;
size_t fsize;

static uint8_t
read (uint8_t pbr, uint16_t pc)
{
    uint32_t addr = (uint32_t)pbr << 16;
    addr |= pc;

    assert (pc < fsize);

    return file[pc]; // FIXME: Should be reading at addr, not curr impl'd
    (void)addr;
}

// FIXME: Make this cycle accurate
static void
reset (w65c832_cpu_t cpu[restrict static 1])
{
    cpu->pbr = 0;
    cpu->pc = 0; // FIXME should actually read reset vector at 0xFFFC,D
}

static void
fetch (w65c832_cpu_t cpu[restrict static 1])
{
    uint8_t opcode = read (cpu->pbr, cpu->pc);
    cpu->pc++;

    cpu->ins = &opcode_matrix[opcode];

    cpu->icycle = 0;
    return;
}

int
main (int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf (stderr, "Improper usage error: no filename provided.\n");
        return EXIT_FAILURE;
    }

    if (u_load_bin_file (argv[1], &file, &fsize) != 0)
    {
        return EXIT_FAILURE;
    }

    bool should_exit = false;
    w65c832_cpu_t cpu;
    reset (&cpu);
    while (!should_exit)
    {
        fetch (&cpu);
        op_print (cpu.ins);
        // See Instruction Operation table for cycle by cycle next steps.
        should_exit = true;
    }

    free (file);
    return EXIT_SUCCESS;
}
