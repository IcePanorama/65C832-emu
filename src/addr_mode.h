#ifndef _W65C832_ADDRESSING_MODE_H_
#define _W65C832_ADDRESSING_MODE_H_

#include <stdint.h>

enum
{
    AM_NAME_MAX_LEN = 32,
    AM_SYM_MAX_LEN = 8,
};

typedef enum addr_mode_e
{
    AM_ABS_INDEXED_INDIR,
    AM_ABS_INDIR,
    AM_ABS_LONG,
    AM_ABS_LONG_IDX,
    AM_ABS_X_IDX,
    AM_ABS_Y_IDX,
    AM_ABSOLUTE,
    AM_ACCUMULATOR,
    AM_BLOCK_MOVE,
    AM_DIR_IDX_INDIR,
    AM_DIR_INDIR,
    AM_DIR_INDIR_IDX,
    AM_DIR_INDIR_LONG,
    AM_DIR_INDIR_LONG_IDX,
    AM_DIR_X_IDX,
    AM_DIR_Y_IDX,
    AM_DIRECT,
    AM_IMMEDIATE,
    AM_IMPLIED,
    AM_PC_REL,
    AM_PC_REL_LONG,
    AM_STACK,
    AM_STACK_REL,
    AM_STACK_REL_INDIR_IDX,
    NADDRMODES
} addr_mode_t;

extern const char am_names[NADDRMODES][AM_NAME_MAX_LEN];
extern const char am_syms[NADDRMODES][AM_SYM_MAX_LEN];
extern const uint8_t am_noperands[NADDRMODES];

addr_mode_t addr_mode_from_string (const char s[static 1]);

int am_init (void);

#endif /* _W65C832_ADDRESSING_MODE_H_ */
