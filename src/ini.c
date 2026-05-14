#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ini.h"

static int load_file_lines (
    const char fpath[restrict static 1],
    char **fdata[restrict static 1],
    size_t nlines[restrict static 1]
);
static int resize_fdata (
    char **fdata[restrict static 1], size_t max_lines[static 1]
);
static int get_lines (
    FILE f[static 1],
    char **fdata[restrict static 1],
    size_t nlines[restrict static 1]
);

int
ini_process_file (const char fpath[restrict static 1], ini_ln_cb_t fn)
{
    int ret = 0;
    size_t nlines = 0;
    char **fdata = NULL;
    if (load_file_lines (fpath, &fdata, &nlines) != 0)
    {
        fprintf (stderr, "Failed to load opcode data from file: %s\n", fpath);
        return -1;
    }
    else
    {
        size_t i;
        for (i = 0; i < nlines; i++)
        {
            ret = (*fn)(fdata[i]);
            if (ret != 0)
            {
                break;
            }
        }
    }

    if (fdata != NULL)
    {
        size_t i;
        for (i = 0; i < nlines; i++)
        {
            free (fdata[i]);
        }
        free (fdata);
    }

    return ret;
}

int
load_file_lines (
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
    char **tmp = realloc (*fdata, sizeof (*tmp) * new_max);
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

int
ini_get_section_title (char * restrict input, char output[restrict static INI_MAX_STR_LEN])
{
    int ret = 0;
    char *tok;

    if (input == NULL)
    {
        return -1;
    }

    tok = strchr (input, '[');
    if ((tok == NULL) || (*tok == '\0'))
    {
        ret = -1;
    }
    else
    {
        const char *sec_name = tok + 1;
        tok = strchr (input, ']');
        if ((tok == NULL) || (*tok == '\0'))
        {
            ret = -1;
        }
        else
        {
            *tok = '\0';
        }

        strncpy (output, sec_name, INI_MAX_STR_LEN);
    }

    return ret;
}


int
ini_get_value (
    char * restrict input, char output[restrict static INI_MAX_STR_LEN])
{
    int ret = 0;
    size_t out_len;
    char *data;

    memset (output, 0, INI_MAX_STR_LEN);

    if (input == NULL)
    {
        return -1;
    }

    data = strchr (input, '=');
    if ((data == NULL) || (*data == '\0'))
    {
        fprintf (
            stderr, "%s - Malformed input (no equal sign): %s", __func__, input
        );
        ret = -1;
    }
    else
    {
        do
        {
            data++;
        }
        while (*data == ' ' && *data != '\0');

        if (*data == '\0')
        {
            fprintf (stderr, "Malformed input (no value): %s", input);
            ret = -1;
        }
        else
        {
        }
    }

    if (data != NULL)
    {
        strncpy (output, data, INI_MAX_STR_LEN);

        out_len = strlen (data) - 1;
        for (data = output + out_len; data >= output; data--)
        {
            if ((*data != '\r') && (*data != '\n'))
            {
                break;
            }

            *data = '\0';
        }
    }

    return ret;
}
