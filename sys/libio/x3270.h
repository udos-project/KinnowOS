#ifndef X3270_H
#define X3270_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <css.h>

enum x3270_order {
    X3270_ORDER_PROGRAM_TAB = 0x05,
    X3270_ORDER_SET_BUFFER_ADDR = 0x11,
    X3270_ORDER_INSERT_CURSOR = 0x13,
    X3270_ORDER_ERASE_UNPROTECTED = 0x12,
    X3270_ORDER_START_FIELD = 0x1D,
    X3270_ORDER_START_FIELD_EXTENDED = 0x29,
    X3270_ORDER_MODIFY_FIELD = 0x2C,
    X3270_ORDER_SET_ATTRIBUTE = 0x28,
    /* RA, takes 2 bytes for address and 1 byte for the character to copy over */
    X3270_ORDER_REPEAT_TO_ADDRESS = 0x3C,
};

/* Extended orders for the 3270 (SFE) */
enum x3270_ext_order {
    /* Character attribute reset, somewhat like \e[0;0m on *NIX */
    X3270_EXT_ORDER_CHARACTER_ATTRIBUTE = 0x00,
    /* Field attribute */
    X3270_EXT_ORDER_FIELD_ATTR = 0xC0,
    /* Extended highlighting (somewhat like text attributes on *NIX) */
    X3270_EXT_ORDER_HIGHLIGHT = 0x41,
    /* Set color of text */
    X3270_EXT_ORDER_COLOR = 0x42,
    /* Symbol set */
    X3270_EXT_ORDER_SYMSET = 0x43,
};

/* Valid highlighting modes for SFE highlight */
enum x3270_highlight {
    X3270_HL_DEFAULT = 0x00,
    X3270_HL_BLINK = 0xF1,
    X3270_HL_REVERSE = 0xF2,
    X3270_HL_UNDERSCORE = 0xF4,
    X3270_HL_INTENSIFY = 0xF8,
};

/* Valid color bytes for SFE Color */
enum x3270_color {
    X3270_COLOR_NEUTRAL_BW = 0xF0,
    X3270_COLOR_CYAN = 0xF1,
    X3270_COLOR_RED = 0xF2,
    X3270_COLOR_PINK = 0xF3,
    X3270_COLOR_GREEN = 0xF4,
    X3270_COLOR_TURQUOISE = 0xF5,
    X3270_COLOR_YELLOW = 0xF6,
    X3270_COLOR_NEUTRAL_WB = 0xF7,
    X3270_COLOR_BLACK = 0xF8,
    X3270_COLOR_BLUE = 0xF9,
    X3270_COLOR_ORANGE = 0xFA,
    X3270_COLOR_LIGHT_GREEN = 0xFB,
    X3270_COLOR_LIGHT_BLUE = 0xFC,
    X3270_COLOR_GRAY = 0xFD,
    X3270_COLOR_LIGHT_GRAY = 0xFE,
    X3270_COLOR_WHITE = 0xFF,
};

#define X3270_ATTR_PARITY (1 << 0), /* Parity bit */
#define X3270_ATTR_PROTECTED (1 << 2) /* Input control, 0 = unprotected, 1 = protected*/
#define X3270_ATTR_UNPROTECTED 0
#define X3270_ATTR_NUMERIC (1 << 3) /* Input mode, 0 = alphanumeric, 1 = numeric */
#define X3270_ATTR_ALPHANUMERIC 0
#define X3270_ATTR(x) ((x) | (1 << 1)) /* Bit 1 is always turned ON */

/* Write control code */
enum x3270_wcc {
    X3270_WCC_PARITY = 0x01,
    X3270_WCC_START_PRINT = 0x10,
    X3270_WCC_SOUND_ALARM = 0x20,
    X3270_WCC_UNLOCK_INPUT = 0x40,
    X3270_WCC_RESET_MDT = 0x80
};

struct x3270_term {
    int fd;
    int (*vert_paging)(struct x3270_term *term);
    struct css_device *dev;
    int cols, rows;
    int x;
    int y;
};

#define PUTC_ARRAY(arr, off, limit, ch) \
    assert((off) < (limit) && (arr) != NULL); \
    arr[(off)++] = (char)(ch);

#define X3270_GOTOXY(term_hdl, buf, offset, n, new_x, new_y) ({ \
        int addr = x3270_to_address((new_x) + ((term_hdl)->cols) * (new_y)); \
        PUTC_ARRAY(buf, *offset, n, X3270_ORDER_SET_BUFFER_ADDR); \
        PUTC_ARRAY(buf, *offset, n, (addr >> 8) & 0xff); \
        PUTC_ARRAY(buf, *offset, n, addr & 0xff); \
        (term_hdl)->x = new_x; \
        (term_hdl)->y = new_y; \
    })

#define X3270_REPEAT(term_hdl, buf, offset, n, end_x, end_y) ({ \
        int addr = x3270_to_address((end_x) + ((term_hdl)->cols) * (end_y)); \
        PUTC_ARRAY(buf, *offset, n, X3270_ORDER_REPEAT_TO_ADDRESS); \
        PUTC_ARRAY(buf, *offset, n, (addr >> 8) & 0xff); \
        PUTC_ARRAY(buf, *offset, n, addr & 0xff); \
    })

#define X3270_SETCOLOR(term_hdl, buf, offset, n, color) ({ \
        PUTC_ARRAY(buf, *offset, n, X3270_ORDER_START_FIELD_EXTENDED); \
        PUTC_ARRAY(buf, *offset, n, 1); \
        PUTC_ARRAY(buf, *offset, n, X3270_EXT_ORDER_COLOR); \
        PUTC_ARRAY(buf, *offset, n, color); \
    })

int x3270_start_term(struct x3270_term *term, struct css_device *dev);
int x3270_to_address(int addr);
uint16_t x3270_get_address(uint16_t addr);
int x3270_clear_screen(struct x3270_term *term);
int x3270_get_input(struct x3270_term *term, char *buf, size_t n);

int x3270_setup_input_field(struct x3270_term *term, char *buf, size_t *offset, size_t n, size_t read_n);
int x3270_puts(struct x3270_term *term, char *buf, size_t *offset, size_t n, const char *str);
int x3270_nputs(struct x3270_term *term, char *buf, size_t *offset, size_t n, const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif
