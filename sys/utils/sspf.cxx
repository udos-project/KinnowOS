/**
 * @file sspf.c
 * @author wxwisiasdf
 * @brief Super SPF Editor
 * @version 0.1
 * @date 2022-06-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <x3270.h>
#include <css.h>

#define SSPF_EDITOR_START_X 6
#define SSPF_EDITOR_START_Y 2

static char tmpbuf[BUFSIZ];
static char *textbuf = nullptr;

void print_screen(struct x3270_term *term)
{
#if 0
    for(int i = SSPF_EDITOR_START_Y; i < term->rows - 1; i++) {
        x3270_gotoxy(term, 1, i); /* Line numbers */
        snprintf(tmpbuf, sizeof(tmpbuf), "%-zu", i);
        x3270_puts(term, tmpbuf);

        x3270_gotoxy(term, SSPF_EDITOR_START_X, i); /* The body */
        x3270_setup_input_field(term, term->cols - SSPF_EDITOR_START_X);
        x3270_nputs(term, &textbuf[(i - SSPF_EDITOR_START_Y) * term->cols], term->cols - SSPF_EDITOR_START_X);
    }
#endif
}

int main(int argc, char **argv)
{
#if 0
    struct css_device *dev = css_get_device(1, 1);
    if(dev == NULL) {
        fprintf(stderr, "Can't open device 1,1\r\n");
        exit(EXIT_FAILURE);
    }

    struct x3270_term term;
    x3270_start_term(&term, dev);
    x3270_clear_screen(&term);

    size_t n_textbuf = term.rows * term.cols;
    textbuf = malloc(n_textbuf);
    if(textbuf == NULL) {
        fprintf(stderr, "Can't create text buffer\r\n");
        exit(EXIT_FAILURE);
    }
    memset(textbuf, ' ', n_textbuf);
    memcpy(&textbuf[0 * term.cols], "* THIS IS SSPF :D", 18);
    memcpy(&textbuf[1 * term.cols], "* THE DEFAULT TEXT EDITOR FOR UDOS!", 36);
    memcpy(&textbuf[2 * term.cols], "* MOSTLY BECAUSE PITUST MADE THE DF EDITOR", 43);
    memcpy(&textbuf[3 * term.cols], "* FOR MINTIA OS, WHICH IS COOL BUT I ALSO", 42);
    memcpy(&textbuf[4 * term.cols], "* WANTED TO BE PART OF THE FUN.", 32);

    x3270_gotoxy(&term, 0, 0);
    x3270_puts(&term, "SSPF V1.0");

    x3270_gotoxy(&term, 0, 1);
    x3270_puts(&term, "Command ==> ");
    x3270_gotoxy(&term, 14, 1);
    x3270_setup_input_field(&term, term.cols - term.x);

    print_screen(&term);

    while(1) {
        size_t n_read = sizeof(tmpbuf);
        int r = x3270_read(&term, tmpbuf, n_read);
        if(r < 0) {
            exit(EXIT_FAILURE);
        } else if(r == 0) {
            continue;
        }

        char *readptr = tmpbuf;
        char *endptr = &tmpbuf[n_read];
        while(*readptr == X3270_ORDER_SET_BUFFER_ADDR) {
            readptr++;
            /* Read address */
            uint16_t addr = x3270_get_address(*(uint16_t *)readptr);
            readptr += 2;

            char inputbuf[BUFSIZ];
            char *inputptr = inputbuf;
            /* Find next SBA and copy to the input buffer */
            while(readptr != endptr && *readptr != X3270_ORDER_SET_BUFFER_ADDR) {
                *(inputptr++) = *(readptr++);
            }

            size_t inputlen = (size_t)((ptrdiff_t)inputptr - (ptrdiff_t)inputbuf);
            uint8_t x = (addr >> 8) & 0xFF;
            uint8_t y = addr & 0xFF;
            if(y == 1) { /* Command */
                char cmdbuf[BUFSIZ];
                /* Command text input starts at 14:1 */
                memcpy(&cmdbuf, inputbuf, inputlen);
            } else if(y >= SSPF_EDITOR_START_Y) { /* Text screen */
                int textaddr = (x - SSPF_EDITOR_START_X) + ((y - SSPF_EDITOR_START_Y) * term.cols);
                /* Text screen starts at 6:2 */
                memcpy(&textbuf[textaddr], inputbuf, inputlen);
                x3270_gotoxy(&term, x, y);
                x3270_nputs(&term, &textbuf[textaddr], inputlen);

                x3270_gotoxy(&term, x, y);
                char tmpbuf[80];
                int offset = 0;
                /* Jump to the original location */
                tmpbuf[offset++] = X3270_ORDER_SET_BUFFER_ADDR;
                addr = x3270_to_address(term.x + (term.y * term.cols));
                tmpbuf[offset++] = (uint8_t)((addr >> 8) & 0xff);
                tmpbuf[offset++] = (uint8_t)(addr & 0xff);
                /* Clear the MODIFY flag */
                tmpbuf[offset++] = X3270_ORDER_START_FIELD_EXTENDED;
                tmpbuf[offset++] = 1;
                tmpbuf[offset++] = X3270_EXT_ORDER_CHARACTER_ATTRIBUTE;
                tmpbuf[offset++] = 0x00;
                memcpy(&tmpbuf[offset], &textbuf[textaddr], inputlen);
                offset += inputlen - 6;
                assert(offset <= sizeof tmpbuf);

                if(write(term.fd, tmpbuf, offset) < 0) {
                    return -1;
                }
            }
        }
        break;
    }

    free(textbuf);
    exit(EXIT_SUCCESS);
#endif
    return 0;
}
