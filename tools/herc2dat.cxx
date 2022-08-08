/// @file herc2data.cxx
/// @brief Converts hercules logs into useful data

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

#define DATA_CHUNK_SIZE 32768

static char *databuf = nullptr;
static size_t n_databuf = DATA_CHUNK_SIZE * 128;
static size_t dataoff = 0;

int main(int argc, char **argv)
{
    databuf = (char *)malloc(n_databuf);
    for(int i = 0; i < argc; i++) {
        if(!strcmp(argv[i], "-dt")) {
            i++;
            if(i >= argc) {
                fprintf(stderr, "Expected an argument for -dt\r\n");
                exit(EXIT_FAILURE);
            }

            if(databuf == NULL || n_databuf == 0) {
                fprintf(stderr, "No data to output");
                exit(EXIT_FAILURE);
            }

            FILE *fp;
            if((fp = fopen(argv[i], "wt")) == NULL) {
                perror("Can't create new file");
                continue;
            }
            if(fwrite(databuf, sizeof(databuf[0]), dataoff, fp) != n_databuf) {
                perror("Can't write to file");
                continue;
            }
            fclose(fp);
        } else {
            FILE *fp;
            if((fp = fopen(argv[i], "rt")) == NULL) {
                perror("Can't open file");
                continue;
            }

            while(!feof(fp)) {
                int hour, minute, second;
                char typebuf[BUFSIZ]; /* General prefix data */
                char argbuf[BUFSIZ]; /* Specific information per type class */
                char tmpbuf[BUFSIZ];

                if(fgets(tmpbuf, sizeof(tmpbuf), fp) == NULL) {
                    perror("Can't read further");
                    continue;
                }
                memset(typebuf, '\0', sizeof(typebuf));
                memset(argbuf, '\0', sizeof(argbuf));
                sscanf(tmpbuf, "%d:%d:%d %s %[^\n]s", &hour, &minute, &second, typebuf, argbuf);
                if(!strncmp(typebuf, "HHC", 3)) {
                    int typenum;
                    char typech;
                    sscanf(&typebuf[3], "%d%c", &typenum, &typech);

                    switch(typenum) {
                    case 2290: {
                        /* r generated dumps look like this
                        * 07:43:16 HHC02290I A:0000000000013000  K:06
                        * 07:43:16 HHC02290I R:0000000000013C50  00000000 00000000 00000000 00000000  ................
                        */
                        unsigned long long int addr;
                        char modech;

                        sscanf(argbuf, "%c:%llx ", &modech, &addr);
                        if(modech == 'A') { /* So far this only prints K:06? */
                            char keytype;
                            int keynum;
                            sscanf(argbuf, "%*c:%*x %c:%d", &keytype, &keynum);
                        } else if(modech == 'R') {
                            char *readptr;
                            int datalen = 0;

                            sscanf(argbuf, "%*c:%*x %[^\n]s", databuf);
                            readptr = databuf;
                            
                            while(1) {
                                /* max number of characters hercules can print */
                                if(datalen >= 4 * 4) {
                                    break;
                                }

                                if(isxdigit(*readptr)) {
                                    char digits[3];
                                    unsigned long num;

                                    /* get the two digits */
                                    digits[0] = *(readptr++);
                                    digits[1] = *(readptr++);
                                    digits[2] = '\0';

                                    num = strtoul(digits, NULL, 16);
                                    databuf[dataoff++] = (char)num;
                                    if(dataoff >= n_databuf) {
                                        n_databuf += DATA_CHUNK_SIZE;
                                        databuf = (char *)realloc(databuf, n_databuf);
                                        if(databuf == NULL) {
                                            fprintf(stderr, "Out of memory, can't allocate databuf of %u bytes", n_databuf);
                                            break;
                                        }
                                    }
                                    datalen++;
                                } else {
                                    /* Hercules uses a single space for separating the data
                                    * and two spaces for separating <key> <data> <repr> */
                                    if(isspace(*readptr)) {
                                        readptr++;
                                        /* this is a repr starting... skip since we already have
                                        * the data */
                                        if(isspace(*readptr)) {
                                            break;
                                        }
                                        continue;
                                    }
                                    break;
                                }
                            }
                        }
                    } break;
                    case 339:
                    case 338:
                    case 337:
                    case 336:
                    case 335:
                    case 334:
                    case 333:
                    case 2915:
                    case 2914:
                    case 1018:
                    case 814:
                    case 107:
                    case 1315:
                    case 1314:
                    case 1313:
                    case 1312:
                    case 1603:
                    case 7:
                    case 100:
                    case 90020:
                    case 17736:
                    case 2385:
                    case 414:
                    case 1474:
                    case 2204:
                    case 17003:
                    case 1024:
                    case 1417:
                    case 17:
                    case 18:
                    case 150:
                    case 151:
                    case 2256:
                    case 109:
                    case 110:
                    case 1413:
                    case 111:
                    case 811:
                    case 1414:
                    case 1415:
                    case 1420:
                    case 1427:
                    case 101:
                    case 417:
                    case 1423:
                    case 1422: {
                        /** @todo hercules HMC console input */
                    } break;
                    default:
                        fprintf(stderr, "Unknown typenum=%d: %s\r\n", typenum, tmpbuf);
                        break;
                    }
                } else {
                    /* Not hercules output */
                    continue;
                }
            }
            fclose(fp);
        }
    }

    if(databuf != NULL) {
        free(databuf);
    }
    exit(EXIT_SUCCESS);
    return 0;
}
