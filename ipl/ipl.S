// This is a gnu assembler re-implementation of PDOS's sapstart.asm
// with a special touch from me, of course ;)

// CCW macros
.macro ccw0 cmd=0, addr=0, flags=0, length=0
    .byte \cmd
    .short (\addr & 0xffff)
    .byte ((\addr & 0xff0000) >> 16)
    .byte \flags
    .byte 0
    .short \length
.endm

.macro ccw1 cmd=0, addr=0, flags=0, length=0
    .byte \cmd
    .byte \flags
    .short \length
    .long \addr
.endm

// ihapsa-like macro definition stuff
#define AMBIT 0x80000000
#define AM64 1

#define FLCCAW 0x48
#define FLCSNPSW 0x60
#define FLCPNPSW 0x68
#define FLCINPSW 0x78
#define FLCMNPSW 0x70
#define FLCIOA 0xB8

#define FLCESNPSW 0x1C0
#define FLCEPNPSW 0x1D0
#define FLCEMNPSW 0x1E0
#define FLCEINPSW 0x1F0

#define RECORD_LENGTH 3450
#define RECORDS_PER_TRACK 4
#define TRACKS_PER_CYLINDER 8
#define NUM_CYLINDERS 5

.section .rodata.init
    // Initial PSW for our IPL
    .long 0x000C0000
    .long (AMBIT + start)

.section .text
.global start
.type start, @function
start:
    // This is the entry point of the IPL program. It will be
    // located at location 8192 - immediately after all the
    // z/Arch hardware reserved fields. In order to support
    // the first (-01) of z/Arch, which IPLed in ESA/390 mode,
    // which means AM64 is not supported, there is no choice
    // but to IPL in AM31 and execute the first few instructions
    // as AM31, then activate z/Arch mode, and only then can a
    // switch to AM64 be done.

    // For the latest z14 the situation is different. It IPLs
    // in z/Arch mode and there is no way to get to ESA/390 mode.
    // Therefore an attempt to switch to z/Arch mode will fail,
    // because it is already in that mode. But that's fine. All
    // you need to do is an unconditional switch to z/Arch and
    // ignore any error.

    // Note that the IPL PSW should ideally have interrupts disabled
    // prior to activating z/Arch mode.

    // Also we have to zero-out r0
    lhi %r0, 0
#if MACHINE >= M_ZARCH
    // Now, activate z/Arch mode
    la %r1, 1  // Magic number for z/Arch
    sigp %r1, 0, 18 // Ignore any non-zero (error) from this

    lhi %r13, start

    // Now we want to point to start2, and then set the low
    // bit to 1, to indicate we want AM64. Simply adding 1
    // will do the trick.
    la %r8, (start2 + 1)

    // We don't care about the old value, so we use register 0
    // to say "unused"
    bsm %r13, %r8
start2:
    // I think these are the the first 64-bits in a 128-bit
    // PSW, and the PSW is split over two locations, but because
    // we use short/compressed PSWs, we don't have to bother
    // with the second half.
#if defined DEBUG
    mvc FLCEINPSW(16), waiter_psw
    mvc FLCEMNPSW(16), waiter_psw
    mvc FLCESNPSW(16), waiter_psw
    mvc FLCEPNPSW(16), waiter_psw
#endif
#else
#if defined DEBUG
    mvc FLCINPSW(8), waiter_psw
    mvc FLCMNPSW(8), waiter_psw
    mvc FLCSNPSW(8), waiter_psw
    mvc FLCPNPSW(8), waiter_psw
#endif
#endif
    // Obtain the IPL device address and save it on r1 because store
    // subchannel uses r1 as device address register which uses the
    // higher half for ID and the lower half for the device number
    lhi %r1, 0
    icm %r1, 0b1111, FLCIOA
#if MACHINE >= M_ZARCH
    lctl 6, 6, all_io_int
#endif
    // R10 stores the "offset" of the buffer
    lhi %r10, 0

    // about 110KB are being read here, more than enough :-)
    lhi %r2, 0 // cyl
    lhi %r3, 1 // head
    lhi %r4, 1 // rec

next_read:
    larl %r5, disk_buffer
    ar %r5, %r10
    
    lhi %r6, RECORD_LENGTH
    brasl %r14, read_block
    ar %r10, %r6 // todo: might be best to use r13 ;)

    ahi %r4, 1 // increment rec
    lhi %r5, RECORDS_PER_TRACK // max records
    cr %r4, %r5
    bne next_read
    lhi %r4, 1 // reset rec

    ahi %r3, 1 // increment head
    lhi %r5, TRACKS_PER_CYLINDER // max heads
    cr %r3, %r5
    bne next_read
    lhi %r3, 0 // reset head

    ahi %r2, 1 // increment cylinders
    lhi %r5, NUM_CYLINDERS // max cylinders
    cr %r2, %r5
    bne next_read
    lhi %r2, 0 // reset cylinders

    j disk_buffer

#if MACHINE >= M_ZARCH
waiter_psw:
    .long 0x00060000 | AM64
    .long AMBIT
    .long 0
    .long (0x00000111)
#else
waiter_psw:
    .long 0x000E0000
    .long (AMBIT + 0x00000111)
#endif

#if MACHINE >= M_S390
.align 4
all_io_int:
    .long 0xFF000000
#endif

.macro CHECK_STATUS r=0
    ipm \r
    srl \r, 28
    cli \r, 3
    be rb_end_error
.endm

// Read a block from the DASD
// r1 = Device number (subchannel id)
// r2 = Cylinder
// r3 = Head
// r4 = Record
// r5 = Buffer
// r6 = Size of buffer
// r13 = Bytes read
read_block:
    // Store from stack
    stcm %r2, 0b0011, rb_cc1 // Cylinder
    stcm %r3, 0b0011, rb_hh1 // Head
    stc %r4, rb_record // Record

    // r5 = buffer, r6 = length
#if MACHINE >= M_S390
    st %r5, rb_ldccw + 4
    sth %r6, rb_ldccw + 2
#else
    stcm %r5, 0b0111, rb_ldccw + 1
    sth %r6, rb_ldccw + 6
#endif

    // Set the I/O PSW (this will jump to the specified label after I/O is done)
#if MACHINE >= M_ZARCH
    mvc FLCEINPSW(16), rb_newio
    stosm FLCEINPSW, 0x00
#else
    mvc FLCINPSW(8), rb_newio
    stosm FLCINPSW, 0x00
#endif

    la %r15, rb_seek
    st %r15, FLCCAW
#if MACHINE < M_S390
    sio 0(%r1)
#else
    la %r15, rb_irb
    tsch 0(%r15)
    CHECK_STATUS %r15

    la %r15, rb_orb
    ssch 0(%r15)
    CHECK_STATUS %r15
#endif
    // On z/Arch the PSW may be out of reach for us so we have to use a base
    // register to bypass the 12-bit limit
    lpsw rb_wtnoer

.align 4
// New IO code will jump here
rb_count:
#if MACHINE >= M_S390
    la %r15, rb_irb
    tsch 0(%r15)
    CHECK_STATUS %r15
    clc 4(4, %r3), rb_end_chain
#else
    clc FLCCSW(4), rb_end_chain
#endif
    be rb_end_success
rb_end_error:
    lhi %r13, 0
    b rb_end
rb_end_success:
    // irb = 0 + 0(scsw) + 10, due to the offset of irb.scsw.count
    // n_read - remainder = actually read
    lr %r13, %r6
    la %r3, rb_irb
    l %r12, 10(%r3)
    sr %r13, %r12
rb_end:
    br %r14

.align 8
rb_wtnoer:
    .long 0x060E0000
    .long AMBIT

.align 8
#if MACHINE >= M_S390
rb_seek:
    ccw1 7, rb_bbcchh, 0x40, 6
rb_search:
    ccw1 0x31, rb_cchhr, 0x40, 5
    ccw1 8, rb_search, 0, 0
rb_ldccw:
    ccw1 0x0e, 0, 0x20, 32767
#else
rb_seek:
    ccw0 7, rb_bbcchh, 0x40, 6
rb_search:
    ccw0 0x31, rb_cchhr, 0x40, 5
    ccw0 8, rb_search, 0, 0
rb_ldccw:
    ccw0 0x0e, 0, 0x20, 32767
#endif
rb_end_chain:
    .long rb_end_chain

#if MACHINE >= M_ZARCH
rb_newio:
    .long 0x00040000 | AM64
    .long AMBIT
    .long 0
    .long (rb_count)
#else
rb_newio:
    .long 0x000C0000
    .long (AMBIT + rb_count)
#endif

.align 2
rb_bbcchh:
    .short 0
rb_cchhr:
rb_cc1:
    .short 0 // 2 uninit bytes
rb_hh1:
    .short 0 // 2 uninit bytes
rb_record:
    .byte 0

#if MACHINE >= M_S390
.align 4
rb_orb:
    .long 0
    .long 0x0080FF00
    .long rb_seek
    .skip (5 * 4) // 5, 4 bytes of 0
.align 4
rb_irb:
    .skip (24 * 4) // 24, 4 bytes of 0
#endif
