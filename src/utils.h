#ifndef _W65C832_EMU_UTILITIES_
#define _W65C832_EMU_UTILITIES_

#include <stddef.h>
#include <stdint.h>

int u_load_bin_file (
    const char fpath[restrict static 1],
    uint8_t *fdata[restrict static 1],
    size_t fsize[restrict static 1]
);

#endif /* _W65C832_EMU_UTILITIES_ */
