#ifndef _W65C832_CPU_H_
#define _W65C832_CPU_H_

#include <stdint.h>

typedef struct w65c832_cpu_s
{
    uint16_t pc;  // Program counter.
} w65c832_cpu_t;

#endif /* _W65C832_CPU_H_ */
