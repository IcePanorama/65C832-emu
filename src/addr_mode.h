#ifndef _W65C832_ADDRESSING_MODE_H_
#define _W65C832_ADDRESSING_MODE_H_

#include <stdint.h>

enum
{
    AM_NAME_MAX_LEN = 32,
    AM_SYM_MAX_LEN = 8,
};

typedef struct addr_mode_s
{
    char name[AM_NAME_MAX_LEN];
    char symbol[AM_SYM_MAX_LEN];
    uint8_t noperands;
} addr_mode_t;

addr_mode_t addr_mode_from_string (const char s[static 1]);

int am_init (void);

#endif /* _W65C832_ADDRESSING_MODE_H_ */
