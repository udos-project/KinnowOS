#ifdef TARGET_S390
.section .init
    /* gcc will nicely put the contents of crtend.o's .init section here. */
    br %r14

.section .fini
    /* gcc will nicely put the contents of crtend.o's .fini section here. */
    br %r14
#elif defined TARGET_OR1K
.section .init
    /* gcc will nicely put the contents of crtend.o's .init section here. */
    l.jr %r14
    l.nop

.section .fini
    /* gcc will nicely put the contents of crtend.o's .fini section here. */
    l.jr %r14
    l.nop
#endif
