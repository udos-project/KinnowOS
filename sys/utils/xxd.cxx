/* xxd.c
 *
 * Small xxd-like program
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++) {
        char tmpbuf[20];
        FILE *fp;

        fp = fopen(argv[i], "r");
        if(fp == NULL) {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }

        while(!feof(fp)) {
            long pos;
            size_t j;

            pos = ftell(fp);
            fread(tmpbuf, sizeof tmpbuf, 1, fp);
            printf("%lx: ", (unsigned long)pos);
            for(j = 0; j < sizeof(tmpbuf); j++) {
                printf("%x ", (unsigned int)tmpbuf[j]);
            }
            printf("\r\n");
        }
        fclose(fp);
    }

    exit(EXIT_SUCCESS);
    return 0;
}
