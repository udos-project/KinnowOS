/* dosfetch.c
 *
 * Neofetch but on UDOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <job.h>

int main(int argc, char **argv)
{
    char tmpbuf[80];
    struct job_stats js;

    printf("DOSFETCH Program v1.0\r\n");

    job_get_stats(&js);
    printf("Memory: %u bytes (%u bytes used)\r\n", js.free_size + js.used_size, js.used_size);
    printf("Fragments: %u regions\r\n", js.n_regions);
    
    exit(EXIT_SUCCESS);
    return 0;
}
