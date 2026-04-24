#include <string.h>

#include "addr_mode.h"

char addr_mode_sym[NADDRESSING_MODES][AM_SYM_LEN] = {
    [AM_ABSOLUTE] = "a",
    [AM_DIR_INDEXED_INDIR] = "(d,x)",
    [AM_DIRECT] = "d",
    [AM_IMPLIED] = "i",
    [AM_IMMEDIATE] = "#",
    [AM_PC_RELATIVE_LONG] = "rl",
    [AM_PC_RELATIVE] = "r",
    [AM_STACK] = "s",
};

const char *addr_mode_names[NADDRESSING_MODES] = {
    [AM_ABSOLUTE] = "AM_ABSOLUTE",
    [AM_DIR_INDEXED_INDIR] = "AM_DIR_INDEXED_INDIR",
    [AM_DIRECT] = "AM_DIRECT",
    [AM_IMPLIED] = "AM_IMPLIED",
    [AM_IMMEDIATE] = "AM_IMMEDIATE",
    [AM_PC_RELATIVE_LONG] = "AM_PC_RELATIVE_LONG",
    [AM_PC_RELATIVE] = "AM_PC_RELATIVE",
    [AM_STACK] = "AM_STACK",
};

#if 0
uint8_t base_noperands[NADDRESSING_MODES] =
{
    [AM_ABSOLUTE] = 2,
    [AM_DIR_INDEXED_INDIR] = 1,
    [AM_DIRECT] = 1,
    [AM_IMPLIED] = 0,
    [AM_IMMEDIATE] = 1,
    [AM_PC_RELATIVE_LONG] = 2,
    [AM_PC_RELATIVE] = 1,
    [AM_STACK] = 0,
};
#endif /* 0 */

addr_mode_t
addr_mode_from_string (const char s[static 1])
{
    size_t i;
    addr_mode_t out = NADDRESSING_MODES;

    for (i = 0; i < NADDRESSING_MODES; i++)
    {
        const char *curr_am_name = addr_mode_names[i];

        if (strncmp (s, curr_am_name, strlen (curr_am_name)) == 0)
        {
            out = (addr_mode_t)i;
            break;
        }
    }

    return out;
}
