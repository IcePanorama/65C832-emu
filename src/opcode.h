#ifndef _W65C832_INSTRUCTION_H_
#define _W65C832_INSTRUCTION_H_

#include <stdbool.h>
#include <stdint.h>

#include "addr_mode.h"

enum
{
    OP_MNEMONIC_LEN = 4,
    NOPCODES = 256,
};

/** Operand size. */
typedef enum operand_sz_e
{
    OPS_FIXED,
    OPS_A,     // depends on A register size
    OPS_XY,    // depends on X/Y register size
    NOPERANDSIZES
} operand_sz_t;

typedef struct opcode_s
{
    uint8_t opcode;
    char mnemonic[OP_MNEMONIC_LEN];
    addr_mode_t *addr_mode;
    operand_sz_t operand_sz;
    uint8_t ncycles;
    bool reserved; // mnemonic = WDM means "Reserved for Future Use"
} opcode_t;

extern opcode_t opcode_matrix[NOPCODES];

int op_init (void);
void op_print (const opcode_t o[restrict static 1]);
uint8_t op_get_noperands (const opcode_t o[static 1]);
operand_sz_t op_get_sz_from_str (const char s[static 1]);

#endif /* _W65C832_INSTRUCTION_H_ */
