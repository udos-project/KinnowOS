/* echo.c
 *
 * Implementation of *nix echo
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;
    for(i = 1; i < argc; i++) {
        puts(argv[i]);
    }
    printf("\r\n");
    fflush(stdout);

    exit(EXIT_SUCCESS);
    return 0;
}
