#ifndef _W65C832_INSTRUCTION_H_
#define _W65C832_INSTRUCTION_H_

#include <stdint.h>

#include "addressing_mode.h"

enum
{
    MNEMONIC_LEN = 4,
    NINSTRUCTIONS = 256,
};

typedef struct instruction_s
{
    uint8_t opcode;
    char mnemonic[MNEMONIC_LEN];
    addressing_mode_t addr_mode;
    uint8_t base_nbytes;
    uint8_t base_ncycles;
} instruction_t;

extern instruction_t opcode_matrix[NINSTRUCTIONS];

int inst_init (void);
void inst_print (instruction_t i[static 1]);

#endif /* _W65C832_INSTRUCTION_H_ */
