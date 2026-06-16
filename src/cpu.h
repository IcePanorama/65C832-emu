#ifndef _W65C832_CPU_H_
#define _W65C832_CPU_H_

#include <stdint.h>

#include "opcode.h"

typedef struct w65c832_cpu_s
{
    uint8_t pbr;  // Program Bank Register
    uint16_t pc;  // Program counter.

    opcode_t *ins;
    uint8_t icycle; // cycle of the current instruction.
} w65c832_cpu_t;

#endif /* _W65C832_CPU_H_ */
