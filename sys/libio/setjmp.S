.section .text
.global _setjmp
_setjmp:
    lhi %r3, 0
    j _sigsetjmp

.global _sigsetjmp
_sigsetjmp:
    
