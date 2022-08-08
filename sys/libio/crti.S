.section .init
.global _init
_init:
    /* gcc will nicely put the contents of crtbegin.o's .init section here. */

.section .fini
.global _fini
_fini:
    /* gcc will nicely put the contents of crtbegin.o's .fini section here. */
