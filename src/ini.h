#ifndef _W65C832_INI_FILE_H_
#define _W65C832_INI_FILE_H_

enum
{
    INI_MAX_STR_LEN = 255,
};

typedef int (*ini_ln_cb_t) (char line[static 1]);

int ini_process_file (const char fpath[restrict static 1], ini_ln_cb_t fn);
int ini_get_section_title (char * restrict input, char output[restrict static INI_MAX_STR_LEN]);
int ini_get_value (char * restrict input, char output[restrict static INI_MAX_STR_LEN]);

#endif /* _W65C832_INI_FILE_H_ */
