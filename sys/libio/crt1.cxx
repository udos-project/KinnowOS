/* crt1.c */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <bits/features.h>
#include <crt0.h>

STDAPI void _start(void)
{
    int r;

    dprintf("CRT0 initialize\r\n");

    if(setlocale(LC_ALL, "C") == nullptr) {
        r = -1;
        goto exit;
    }

    /* Open the default standard I/O streams */
    if((stdin = fopen("/CSUP.IN", "r")) == nullptr) {
        r = -2;
        goto exit;
    }
    if((stdout = fopen("/CSUP.OUT", "w")) == nullptr) {
        r = -3;
        goto exit;
    }
    if((stderr = fopen("/CSUP.ERR", "w")) == nullptr) {
        r = -4;
        goto exit;
    }
    if((stdprn = fopen("/CSUP.PRN", "w")) == nullptr) {
        r = -5;
        goto exit;
    }

    /* Query the PDB */
    io_svc(SVC_GET_PDB, (uintptr_t)&pdb_area, 0, 0);

    /** @todo Obtain and parse environment and arguments from PDB */

    /** @todo Initialize the global constructors on crtni.asm */
    /*_init();*/

    /** @todo main() might not nescesarily return a value */
    r = main(0, nullptr, nullptr);
exit:
    _exit((r >= 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}

STDAPI void _exit(int status)
{
    /* Close all the user files - this also includes STDOUT, STDIN and etc */
    for(size_t i = 0; i < FOPEN_MAX; i++) {
        if(_files[i].handle != nullptr) {
            fclose(&_files[i]);
        }
    }
    _fini();

    io_svc(SVC_ABEND, (uintptr_t)status, 0, 0);
}
