#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addr_mode.h"
#include "ini.h"
#include "utils.h"

enum
{
    ADDR_MODE_MAP_SIZE = 36,
};

typedef enum am_df_type_e
{
    AMDFT_SYM, AMDFT_NOPERANDS, NAMDFTYPES
} am_df_type_t;

addr_mode_t am_map[ADDR_MODE_MAP_SIZE];

static const char *am_fname = "addr_modes.ini";
static const uint8_t am_uninitd_val = UINT8_C(0xFF);
static const char *df_toks[NAMDFTYPES] = {
    [AMDFT_SYM] = "sym", [AMDFT_NOPERANDS] = "noperands"
};

static int process_line (char line[restrict static 1]);
static int insert_new_am (const addr_mode_t am[restrict static 1]);
static size_t am_hash_name (const char name[restrict static 1]);
static int proc_new_am (char in[restrict static 1], char new_am_name[restrict static AM_NAME_MAX_LEN]);
static int proc_df (char in[restrict static 1], addr_mode_t am[restrict static 1]);
static am_df_type_t get_df_type (char in[restrict static 1]);
static int proc_sym_df (const char value[restrict static 1], addr_mode_t am[restrict static 1]);
static int proc_noperands_df (const char value[restrict static 1], addr_mode_t am[restrict static 1]);

int
am_init (void)
{
    memset (am_map, am_uninitd_val, sizeof (addr_mode_t) * ADDR_MODE_MAP_SIZE);

    if (ini_process_file (am_fname, &process_line) != 0)
    {
        return -1;
    }

    return 0;
}

int
process_line (char line[restrict static 1])
{
    static addr_mode_t curr = { 0 };
    static bool is_new_am = true;
    char cpy[256];
    int ret = 0;
    char am_name[AM_NAME_MAX_LEN] = { 0 };

    strncpy (cpy, line, strlen (line) + 1);

    if (is_new_am)
    {
        if (proc_new_am (cpy, am_name) != 0)
        {
            fprintf (stderr, "Malformed input: %s\n", line);
            ret = -1;
        }
        else
        {
            strncpy (curr.name, am_name, AM_NAME_MAX_LEN);
            is_new_am = false;
        }
    }
    else
    {
        if ((*cpy == '\n') || (*cpy == '\r'))
        {
            if (insert_new_am (&curr) != 0)
            {
                ret = -1;
            }
            else
            {
                memset (&curr, am_uninitd_val, sizeof (addr_mode_t));
                is_new_am = true;
            }
        }
        else if (proc_df (cpy, &curr) != 0)
        {
            ret = -1;
        }
    }

    return ret;
}

int
proc_new_am (
    char in[restrict static 1],
    char new_am_name[restrict static AM_NAME_MAX_LEN]
)
{
    int ret = 0;
    char out[INI_MAX_STR_LEN] = { 0 };

    ret = ini_get_section_title (in, out);
    if (ret == 0)
    {
        const size_t name_len = strlen (out) + 1;

        if (name_len > AM_NAME_MAX_LEN)
        {
            fprintf (
                stderr,
                "Given addressing mode name is too long: %zu vs %zu",
                name_len,
                (size_t)AM_NAME_MAX_LEN
            );

            ret = -1;
        }
        else
        {
            strncpy (new_am_name, out, AM_NAME_MAX_LEN);
        }
    }

    return ret;
}

void
am_print (const addr_mode_t am[restrict static 1])
{
    fprintf (stdout,
        "%s { symbol = %s, noperands = %"PRIu8" } \n",
        am->name,
        am->symbol,
        am->noperands
    );
}

int
insert_new_am (const addr_mode_t am[restrict static 1])
{
    size_t new_idx = am_hash_name (am->name);
    addr_mode_t *new = NULL;
    int ret = 0;
    size_t try;

    for (try = 0; try < ADDR_MODE_MAP_SIZE; try++)
    {
        addr_mode_t *curr = &am_map[new_idx];
        if (*((uint8_t *)curr) == am_uninitd_val)
        {
            new = curr;
            break;
        }
        else
        {
            new_idx = (new_idx + 1) % ADDR_MODE_MAP_SIZE;
        }
    }

    if (new == NULL)
    {
        fprintf (stderr, "Addr mode hash map full!\n");
        ret = -1;
    }
    else
    {
        memcpy (new, am, sizeof (addr_mode_t));
    }

    return ret;
}

size_t
am_hash_name (const char name[restrict static 1])
{
    size_t hash = (size_t)UINTMAX_C(5381);

    for (char c = *name; c != '\0'; c++)
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % ADDR_MODE_MAP_SIZE;
}

int
proc_df (char in[restrict static 1], addr_mode_t am[restrict static 1])
{
    const am_df_type_t type = get_df_type (in);
    int ret = 0;
    char value[INI_MAX_STR_LEN] = { 0 };

    if (ini_get_value (in, value) != 0)
    {
        fprintf (stderr, "Failed to read value in line: %s", in);
        ret = -1;
    }
    else
    {
        switch (type)
        {
            case AMDFT_SYM:
                if (proc_sym_df (value, am) == 0)
                {
                    break;
                }

                fprintf (
                    stderr,
                    "Failed to process symbol for addr mode: %s\n",
                    am->name
                );
                ret = -1;
                break;
            case AMDFT_NOPERANDS:
                if (proc_noperands_df (value, am) == 0)
                {
                    break;
                }

                fprintf (
                    stderr,
                    "Failed to process symbol for addr mode: %s\n",
                    am->name
                );
                ret = -1;
                break;
            default:
                assert (false); // should be impossible to reach here.
        }
    }

    return ret;
}

am_df_type_t
get_df_type (char in[restrict static 1])
{
    am_df_type_t out = NAMDFTYPES;
    for (size_t i = 0; i < NAMDFTYPES; i++)
    {
        const char *curr_tok = df_toks[i];
        if (strncmp (curr_tok, in, strlen (curr_tok)) == 0)
        {
            out = (am_df_type_t)i;
            break;
        }
    }

    return out;
}

int
proc_sym_df (
    const char value[restrict static 1], addr_mode_t am[restrict static 1])
{
    const size_t val_len = strlen (value) + 1;
    int ret = 0;

    if (val_len > AM_SYM_MAX_LEN)
    {
        ret = -1;
    }
    else
    {
        strncpy (am->symbol, value, AM_SYM_MAX_LEN);
    }

    return ret;
}

int
proc_noperands_df (
    const char value[restrict static 1], addr_mode_t am[restrict static 1])
{
    uintmax_t nops = 0;
    int ret = 0;

    for (char *c = (char *)value; *c != '\0'; c++)
    {
        if (('0' <= *c) && (*c <= '9'))
        {
            nops *= 10;
            nops += *c - '0';
        }
        else
        {
            fprintf (stderr, "Decimal value expected, got: %s\n", value);
            ret = -1;
        }
    }

    if (ret == 0)
    {
        if (nops > UINT8_MAX)
        {
            fprintf (
                stderr,
                "Value exceeds limit: %"PRIdMAX" vs %"PRId8"\n", 
                nops,
                UINT8_MAX
            );
            ret = -1;
        }
        else
        {
            am->noperands = (uint8_t)nops;
        }
    }

    return ret;
}

addr_mode_t *
addr_mode_from_string (const char s[static 1])
{
    const size_t slen = strlen (s);
    size_t idx = am_hash_name (s);
    addr_mode_t *out = NULL;
    addr_mode_t *curr = &am_map[idx];
    size_t try;

    for (try = 0; try < ADDR_MODE_MAP_SIZE; try++)
    {
        if ((*(uint8_t *)curr) == am_uninitd_val)
        {
            break;
        }
        else if (strncmp (s, curr->name, slen) == 0)
        {
            out = curr;
            break;
        }

        idx = (idx + 1) % ADDR_MODE_MAP_SIZE;
        curr = &am_map[idx];
    }

    return out;
}

