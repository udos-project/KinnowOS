/* cat.c
 *
 * Implementation of *nix cat
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp = stdin;
    if(argc >= 2) {
        if((fp = fopen(argv[1], "r")) == NULL) {
            perror("can't open file");
            exit(EXIT_FAILURE);
        }
    } else {
        fp = stdin;
    }

    while(!feof(fp)) {
        putchar(fgetc(fp));
    }
    
    fclose(fp);
    exit(EXIT_SUCCESS);
    return 0;
}
