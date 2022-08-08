/* uname.c
 *
 * Simple uname implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int i;
    
    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-r")) {
            printf("1.0\r\n");
        } else if(strcmp(argv[i], "-a")) {
            printf("s390x\r\n");
        }
    }

    printf("udos\r\n");
    exit(EXIT_SUCCESS);
    return 0;
}
