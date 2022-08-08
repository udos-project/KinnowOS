#include <setjmp.h>
#include <stdio.h>
#include <bits/features.h>

STDAPI void _longjmp(jmp_buf env, int val)
{
    dprintf("_longjmp(%p, %i);\r\n", &env[0], val);
    while(1);
}
