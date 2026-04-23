#ifndef _W65C832_ADDRESSING_MODE_H_
#define _W65C832_ADDRESSING_MODE_H_

enum
{
    ADDRESSING_MODE_SYM_LEN = 8
};

typedef enum addressing_mode_e
{
    AM_ABSOLUTE,
    AM_DIR_INDEXED_INDIR,
    AM_DIRECT,
    AM_IMPLIED,
    AM_IMMEDIATE,
    AM_PC_RELATIVE,
    AM_PC_RELATIVE_LONG,
    AM_STACK,
    NADDRESSING_MODES
} addressing_mode_t;

extern char addr_mode_sym[NADDRESSING_MODES][8];

addressing_mode_t addr_mode_from_string (const char s[static 1]);

#endif /* _W65C832_ADDRESSING_MODE_H_ */
