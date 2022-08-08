/* hello.c
 *
 * Hello World demostration of LIBTUI and LIBIO
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x3270.h>
#include <css.h>

static void draw_strip(struct x3270_term *term, int color, int y)
{
    int addr;
    size_t offset;
    char tmpbuf[32];

    offset = 0;
    tmpbuf[offset++] = X3270_WCC_SOUND_ALARM;

    tmpbuf[offset++] = X3270_ORDER_SET_BUFFER_ADDR;
    addr = x3270_to_address(79 * y);
    tmpbuf[offset++] = (uint8_t)((addr >> 8) & 0xff);
    tmpbuf[offset++] = (uint8_t)(addr & 0xff);

    tmpbuf[offset++] = X3270_ORDER_START_FIELD_EXTENDED;
    tmpbuf[offset++] = 2;
    tmpbuf[offset++] = X3270_EXT_ORDER_COLOR;
    tmpbuf[offset++] = (uint8_t)color;
    tmpbuf[offset++] = X3270_EXT_ORDER_HIGHLIGHT;
    tmpbuf[offset++] = X3270_HL_REVERSE;

    tmpbuf[offset++] = X3270_ORDER_REPEAT_TO_ADDRESS;
    addr = x3270_to_address(79 * (y + 1));
    tmpbuf[offset++] = (uint8_t)((addr >> 8) & 0xff);
    tmpbuf[offset++] = (uint8_t)(addr & 0xff);
    tmpbuf[offset++] = 'A';
    write(term->fd, tmpbuf, offset);
}

const char *ver_msg = "uDOS version 1.0"/*"uDOSバージョン1.0"*/;
int main(int argc, char **argv)
{
    struct css_device *dev;
    struct x3270_term term;
    char tmpbuf[80];

    dev = css_get_device(1, 1);
    if(dev == NULL) {
        printf("Can't open device 1,1\r\n");
        exit(EXIT_FAILURE);
    }

    x3270_start_term(&term, dev);
    x3270_clear_screen(&term);
    x3270_get_input(&term, tmpbuf, sizeof(tmpbuf));

    exit(EXIT_SUCCESS);
    return 0;
}
