#ifndef _W65C832_ADDRESSING_MODE_H_
#define _W65C832_ADDRESSING_MODE_H_

enum
{
    ADDRESSING_MODE_SYM_LEN = 8
};

typedef enum addressing_mode_e
{
    AM_IMPLIED,
    NADDRESSING_MODES
} addressing_mode_t;

extern char addressing_mode_sym[NADDRESSING_MODES][8];

#endif /* _W65C832_ADDRESSING_MODE_H_ */
