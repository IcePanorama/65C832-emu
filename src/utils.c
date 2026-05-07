#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static int get_file_size (FILE fptr[restrict static 1], size_t size[restrict static 1]);
static int file_to_byte_array (FILE f[restrict static 1], const size_t fsize, uint8_t *restrict out);
static int resize_fdata (char **fdata[restrict static 1], size_t max_lines[static 1]);
static int get_lines (FILE f[static 1], char **fdata[restrict static 1], size_t nlines[restrict static 1]);

int
u_load_bin_file (
    const char fpath[restrict static 1],
    uint8_t *fdata[restrict static 1],
    size_t fsize[restrict static 1]
)
{
    FILE *fptr = fopen(fpath, "rb");
    size_t size_b; // in bytes
    int ret = 0;

    if (!fptr)
    {
        fprintf (stderr, "Failed to open file: %s\n", fpath);
        ret = -1;
    }
    else if (get_file_size (fptr, &size_b) != 0)
    {
        fprintf (stderr, "Failed to determine size of file: %s\n", fpath);
        ret = -1;
    }
    else
    {
        uint8_t *file = malloc (size_b * sizeof (uint8_t));
        if (file == NULL)
        {
            fprintf (stderr, "%s: Out of memory error.\n", __func__);
            ret = -1;
        }
        else if (file_to_byte_array (fptr, size_b, file) != 0)
        {
            fprintf (stderr, "Error reading file: %s\n", fpath);
            free (file);
            file = NULL;
            ret = -1;
        }
        else
        {
            *fdata = file;
            *fsize = (size_t)size_b;
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

int
u_load_file_lines (
    const char fpath[restrict static 1],
    char **fdata[restrict static 1],
    size_t nlines[restrict static 1]
)
{
    FILE *fptr = NULL;
    char **out;
    int ret = 0;

    fptr = fopen(fpath, "r");
    if (fptr == NULL)
    {
        fprintf (stderr, "Failed to open file: %s\n", fpath);
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

int
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
