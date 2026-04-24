#ifndef _W65C832_ADDRESSING_MODE_H_
#define _W65C832_ADDRESSING_MODE_H_

#include <stdint.h>

enum
{
    AM_SYM_LEN = 8
};

typedef enum addr_mode_e
{
    AM_ABSOLUTE,
    AM_DIR_INDEXED_INDIR,
    AM_DIRECT,
    AM_IMPLIED,
    AM_IMMEDIATE,
    AM_PC_RELATIVE_LONG,
    AM_PC_RELATIVE,
    AM_STACK,
    NADDRESSING_MODES
} addr_mode_t;

extern char addr_mode_sym[NADDRESSING_MODES][AM_SYM_LEN];

addr_mode_t addr_mode_from_string (const char s[static 1]);

#endif /* _W65C832_ADDRESSING_MODE_H_ */
