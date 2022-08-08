/* cp.c
 *
 * *nix copy, cp [src] [dest]
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <job.h>
#include <css.h>

int main(int argc, char **argv)
{
    FILE *in, out;
    if(argc <= 1 + 2) {
        fprintf(stderr, "No arguments specified\r\n");
    }

    /** @todo Copy */

    exit(EXIT_SUCCESS);
    return 0;
}
