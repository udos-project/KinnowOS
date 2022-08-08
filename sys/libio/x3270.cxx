#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <x3270.h>
#include <css.h>
#include <svc.h>

STDAPI int x3270_start_term(struct x3270_term *term, struct css_device *dev)
{
    term->dev = dev;
    term->cols = 80;
    term->rows = 23;

    dprintf("Enabling terminal\r\n");
    term->fd = open("/SYSTEM/DEVICES/TERM000", O_RDWR);
    if(term->fd < 0) {
        return -1;
    }
    return term->fd;
}

static const unsigned char ebcdic_map[64] = {
    0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x7B,
    0x7C, 0x7D, 0x7E, 0x7F
};

STDAPI int x3270_to_address(int addr)
{
    uint8_t tmp[2];
    /* 12-bit conversion does not need to be done since it's higher than the
     * imposed 4K limit */
    if(addr >= 0xfff) {
        tmp[0] = (uint8_t)((addr >> 8) & 0x3F);
        tmp[1] = (uint8_t)(addr & 0x3F);
        return (int)(*((uint16_t *)&tmp[0]));
    }
    /* Take only in account low 6-bits */
    tmp[0] = ebcdic_map[(addr >> 6) & 0x3F];
    tmp[1] = ebcdic_map[addr & 0x3F];
    return (int)(*((uint16_t *)&tmp[0]));
}

STDAPI uint16_t x3270_get_address(uint16_t addr)
{
    uint8_t a = (uint8_t)((addr >> 8) & 0xFF);
    uint8_t b = (uint8_t)(addr & 0xFF);
    return !(a & 0xC0) ? ((a & 0x3F) << 8) | b : ((a & 0x3F) << 6) | (b & 0x3F);
}

STDAPI int x3270_clear_screen(struct x3270_term *term)
{
    char tmpbuf[8];
    int addr;

    /* Clear the entire screen :) */
    tmpbuf[0] = X3270_ATTR(0);

    tmpbuf[1] = X3270_ORDER_SET_BUFFER_ADDR;
    addr = x3270_to_address(0);
    tmpbuf[2] = (uint8_t)((addr >> 8) & 0xff);
    tmpbuf[3] = (uint8_t)(addr & 0xff);

    tmpbuf[4] = X3270_ORDER_REPEAT_TO_ADDRESS;
    addr = x3270_to_address(0);
    tmpbuf[5] = (uint8_t)((addr >> 8) & 0xff);
    tmpbuf[6] = (uint8_t)(addr & 0xff);
    tmpbuf[7] = ' ';
    if(write(term->fd, tmpbuf, sizeof(tmpbuf)) < 0) {
        return -1;
    }
    return 0;
}

STDAPI int x3270_get_input(struct x3270_term *term, char *buf, size_t n)
{
    char tmpbuf[80];
    size_t offset = 0;
    int addr;

    if(n >= sizeof(tmpbuf) - 8) {
        n = sizeof(tmpbuf) - 8;
    }

    x3270_setup_input_field(term, tmpbuf, &offset, sizeof(tmpbuf), n);
    if(read(term->fd, buf, n) < 0) {
        return -1;
    }

    n -= 6; /* Skip first characters from reading */
    memmove(buf, &buf[6], n);
    memset(&buf[n], 0, 6);

    /* Now go back to the field, and clear it from the MODIFY flag */
    offset = 0;
    PUTC_ARRAY(tmpbuf, offset, sizeof(tmpbuf), X3270_ATTR(0));
    X3270_REPEAT(term, tmpbuf, &offset, n, term->x, term->y); /* Jump to the original location */
    PUTC_ARRAY(tmpbuf, offset, sizeof(tmpbuf), X3270_ORDER_START_FIELD_EXTENDED);
    PUTC_ARRAY(tmpbuf, offset, sizeof(tmpbuf), 1);
    PUTC_ARRAY(tmpbuf, offset, sizeof(tmpbuf), X3270_EXT_ORDER_CHARACTER_ATTRIBUTE);
    PUTC_ARRAY(tmpbuf, offset, sizeof(tmpbuf), 0); /* Clear the MODIFY flag */
    memcpy(&tmpbuf[offset], buf, n);
    if(write(term->fd, tmpbuf, 8 + n) < 0) {
        return -1;
    }
    return 0;
}

STDAPI int x3270_setup_input_field(struct x3270_term *term, char *buf, size_t *offset, size_t n, size_t read_n)
{
    assert(*offset == 0); /* Offset must be zero */

    /* Obtain - release the keyboard lock from the 3270 to allow the operator
     * to input whatever they please (with protected fields of course!) */
    PUTC_ARRAY(buf, *offset, n, X3270_ATTR(X3270_WCC_UNLOCK_INPUT | X3270_WCC_RESET_MDT));
    X3270_GOTOXY(term, buf, offset, n, term->x, term->y);
    PUTC_ARRAY(buf, *offset, n, X3270_ORDER_START_FIELD);
    PUTC_ARRAY(buf, *offset, n, 0);
    PUTC_ARRAY(buf, *offset, n, X3270_ORDER_INSERT_CURSOR); /* Insert cursor at start of field */
    X3270_REPEAT(term, buf, offset, n, term->x + (int)read_n, term->y);
    PUTC_ARRAY(buf, *offset, n, ' ');
    return 0;
}

STDAPI int x3270_puts(struct x3270_term *term, char *buf, size_t *offset, size_t n, const char *str)
{
    char change_addr = 0;
    size_t i, len = strlen(str);

    assert(term != NULL && term->y < term->rows);

    /* We don't really have to track what we have printed, we can treat this device like a printer */
    PUTC_ARRAY(buf, *offset, n, X3270_ATTR(0));
    X3270_GOTOXY(term, buf, offset, n, term->x, term->y);

    /** @todo Properly set $SBA when newlines appear **midtext** */
    /* Text parsing */
    for(i = 0; i < len; i++) {
        if(str[i] == '\n') {
            term->y++;
            change_addr = 1;

            /* Leave terminal vertical scrolling up to the application */
            if(term->y >= term->rows - 1) {
                /* Write old buffer and then return */
                if(write(term->fd, buf, *offset) < 0) {
                    return -1;
                }

                /* Call the vertical paging handler if any */
                if(term->vert_paging != NULL) {
                    term->vert_paging(term);
                }

                *offset = 0;
                PUTC_ARRAY(buf, *offset, n, X3270_ATTR(0));
            }
        } else if(str[i] == '\r') {
            term->x = 0;
            change_addr = 1;
        } else {
            /* Update address once a printable character appears */
            if(change_addr) {
                X3270_GOTOXY(term, buf, offset, n, term->x, term->y);
                change_addr = 0;
            }

            PUTC_ARRAY(buf, *offset, n, str[i]);
            term->x++;
            if(term->x >= term->cols) {
                term->x = 0;
                term->y++;
                change_addr = 1;
            }
        }
    }
    return 0;
}

STDAPI int x3270_nputs(struct x3270_term *term, char *buf, size_t *offset, size_t n, const char *str, size_t len)
{
    char tmpbuf[BUFSIZ];
    memcpy(tmpbuf, str, len);
    tmpbuf[len] = '\0';
    return x3270_puts(term, buf, offset, n, tmpbuf);
}
