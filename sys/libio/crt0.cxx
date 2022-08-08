/* crt0.c */
/* Library description */
#include <ldesc.h>
#include <bits/features.h>
#include <crt0.h>

const struct ldesc_mod __udos_mod_desc = {
    .en_name = "I/O Library",
    .jp_name = "I/O Library" /*"I/Oライブラリ"*/,
    .license = LDESC_LICENSE_UNLICENSE,
    .start = &_start,
    .exit = &_exit
};
