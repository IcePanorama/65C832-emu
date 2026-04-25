#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "opcode.h"

enum
{
    INPUT_FILE_SIZE_B = 46,
};

static int load_file (const char path[restrict static 1], uint8_t *data[restrict static 1], size_t size[restrict static 1]);
static int get_file_size (FILE fptr[restrict static 1], size_t size[restrict static 1]);
static int file_to_byte_array (FILE f[restrict static 1], const size_t fsize, uint8_t *restrict out);
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

    if (load_file (argv[1], &file, &fsize) != 0)
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

int
load_file (
    const char path[restrict static 1],
    uint8_t *data[restrict static 1],
    size_t size[restrict static 1]
)
{
    FILE *fptr = fopen(path, "rb");
    size_t fsize;
    int ret = 0;

    if (!fptr)
    {
        fprintf (stderr, "Failed to open file: %s\n", path);
        ret = -1;
    }
    else if (get_file_size (fptr, &fsize) != 0)
    {
        fprintf (stderr, "Failed to determine size of file: %s\n", path);
        ret = -1;
    }
    else
    {
        uint8_t *file = malloc (fsize * sizeof (uint8_t));
        if (file == NULL)
        {
            fprintf (stderr, "%s: Out of memory error.\n", __func__);
            ret = -1;
        }
        else if (file_to_byte_array (fptr, fsize, file) != 0)
        {
            fprintf (stderr, "Error reading file: %s\n", path);
            free (file);
            file = NULL;
            ret = -1;
        }
        else
        {
            *data = file;
            *size = (size_t)fsize;
        }
    }

    if (fptr)
    {
        fclose (fptr);
    }

    return ret;
}

int
get_file_size (FILE fptr[restrict static 1], size_t size[restrict static 1])
{
    int ret = fseek (fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    if ((ret != 0) || (fsize == -1))
    {
        ret = -1;
    }
    else
    {
        *size = (size_t)fsize;
    }

    rewind (fptr);
    return ret;
}

int
file_to_byte_array (
    FILE f[restrict static 1], const size_t fsize, uint8_t *restrict out)
{
    size_t total_nbytes = 0;
    int ret = 0;

    while (total_nbytes < fsize)
    {
        size_t rem = fsize - total_nbytes;
        size_t n = fread (out + total_nbytes, sizeof (uint8_t), rem, f);

        if (n == 0)
        {
            if (feof (f))
            {
                break;
            }
            else if (ferror (f))
            {
                ret = -1;
                break;
            }
        }
        total_nbytes += n;
    }

    if (total_nbytes != fsize)  // handle early feof case
    {
        ret = -1;
    }

    return ret;
}
