/* reldb.c
 *
 * Relational Database System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION_STRING "v1.0"

struct db_row {

};

struct db_data {
    char *name;
};

struct db_column {
    struct db_data *data;
    size_t n_data;
};

struct db_table {
    struct db_row *rows;
    size_t n_rows;
    struct db_column *columns;
    size_t n_columns;
};

int main(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "/VERSION")) {
            printf("UDOS DATABASE Daemon " VERSION_STRING "\r\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Unknown option %s\r\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    /** @todo Run as daemon */
    while(1) {

    }
    exit(EXIT_SUCCESS);
    return 0;
}
