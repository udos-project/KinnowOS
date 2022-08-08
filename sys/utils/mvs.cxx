#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

struct s390_psw {

};

// MVS support
//
// MVS is a little bit weird when it comes to a consistent API, as it was mostly made
// a little bit unconsistent; albeit it's not my problem and I have to support it nonethless.
// 
// Offset       Description
// +16          CVT address
// ....+772         Address of the systable
// ........+160         LX/EX for obtain
// ........+204         LX/EX for release
// ........+232         Track calc
// These use the following structure for LX/EX requests (both obtain/release)
// DC A(0)              STORAGE LENGTH
// DC BL1'00000000'
// DC AL1(0)            KEY
// DC AL1(0)            SUBPOOL
// DC BL1'00000000'     FLAGS
//        ^^^^^^^^
//        |||||||`----- Perform release(?)
//        ||||||`------ ???
//        |||||`------- ???
//        ||||`-------- ???
//        |||`--------- ???
//        ||`---------- ???
//        |`----------- ???
//        `------------ ???
// +540         Pointer to current TCB
struct mvs_systable {
    uint8_t padding1[160]; /* 0-160 */
    uint32_t lxex_obtain; /* 160-164 */
    uint8_t padding2[36]; /* 164-200 */
    uint32_t lxex_release; /* 200-204 */
    uint8_t padding3[28]; /* 204-232 */
    uint32_t trkcalc; /* 232-236 */
} __attribute__((packed));

/** @todo Properly declare the structures below, since they are taken
 * from z/PDOS, which does not use fixed-width types, instead relying on
 * the compiler using the proper width
 */

// Event control block
typedef int32_t mvs_ecb_t;

struct mvs_task {
    int32_t abend;
};

struct mvs_cdentry {
    uint8_t filler1[8];
    uint8_t cdname[8]; // Name of module
    uint32_t cdentpt; // Entry point of module
};

/* note that RB has another 64 bytes in a prefix area */
struct mvs_rb {
    uint8_t filler1[12];
    uint32_t rbcde1; // This is actually a 3-byte address
    // The following stuff is not in correct order/place
    uint32_t regs[16];
    struct s390_psw psw;
    uint32_t rblinkb; // This is really 3 bytes
    mvs_ecb_t postecb;
    uint32_t next_exe; // Next executable's location
    uint8_t savearea[20 * 4]; // Needs to be in user space
    uint32_t pptrs[1];
};

struct mvs_tioentry {
    char tioelngh;
};

struct mvs_tiot {
    char unused[24];
    struct mvs_tioentry tioentry;
};

/* note that the TCB has another 32 bytes in a prefix area */
struct mvs_tcb {
    uint32_t tcbrbp;
    uint8_t unused1[8];
    uint32_t tcbtio;
    int32_t tcbcmp;
};

struct mvs_ascb {
    uint8_t filler1[108];
    uint32_t ascbasxb;
};

/* Dataset Control Block */
struct mvsabi_dcb {
    /* Direct access descriptor */
    uint8_t dvtb[16];
    uint32_t keylen;
    /* Common access member */
    uint8_t bufno;
    uint8_t bufcb[3];
    uint16_t bufl;
    uint16_t dso;
    uint32_t iobad;
    /* Foundation extern */
    uint8_t bftek;
    uint8_t eodad[3];
    uint8_t recfm;
    uint8_t exlst[3];
    /* Foundation block */
    uint8_t ddname[8];
    uint8_t open_flags;
    uint8_t iosflags;
    uint16_t mac;
    /* BSAM-BPAM-QSAM interface */
    uint8_t optcd;
    uint8_t check[3];
    uint32_t synad;
    uint16_t internal1;
    uint16_t blksize;
    uint32_t internal2;
    uint32_t internal3;
    /* QSAM interface */
    uint32_t eobad;
    uint32_t recad;
    uint16_t qsws;
    uint16_t lrecl;
    uint8_t eropt;
    uint8_t ctrl[3];
    uint32_t reserved;
    uint32_t eob;
} __attribute__((packed));

/// @todo Some way to register SVC HANDLER routers
#if 0
    /* MVS Compatibility */
    switch(code) {
    /* GETMAIN
     * R0 is the lenght of the requested storage area
     * R1 is the returned storage area
     */
    case 10: {
        void *p;
        p = storage::alloc((size_t)frame->r0);
        frame->r15 = (p == nullptr) ? 1 : 0;
        frame->r1 = (arch_dep::register_t)p;
    } break;
    /* ABEND
     * R1 has the ABEND code
     */
    case 13: {
        kpanic("No ABEND implemented");
    } break;
    /* OPEN FILE */
    case 19: {
        /* R1 has the address to the DCB_SVC19 */
        typedef struct {
            uint8_t options;
            uint16_t low_addr;
            uint8_t hi_addr;
        } open_dcb_desc; /* open dcb descriptor */
        kpanic("No OPEN DCB implemented");
    } break;
    case 120: {
        /* High order bit is set for GETMAIN, otherwise it's a FREEMAIN */
        /* VRU GETMAIN
        * Variable request of a block of memory
        * R0 has the minimum size
        * R1 has the maximum size
        * R15 has the address of the VRU/returns the success indiciation
        */
        if((int)frame->r1 < 0) {
            typedef struct {
                uint32_t max_len;
                uint32_t min_len;
                uint8_t reserved[3];
                uint8_t mode;
            } getmain_vru; /* variable request */
            void *p = nullptr;
            size_t size;

            if(frame->r1 < frame->r0) {
                kpanic("Minimum > Maximum");
            }

            /* Use minimum size by default */
            /** @todo Actually use as much as possible but keeping stuff straight */
            size = (size_t)frame->r0;
            if(size) {
                p = storage::alloc(size);
                frame->r1 = (arch_dep::register_t)p;
                /* "If you GETMAIN more than 8192 bytes or request a multiple of 4096 the system will clear the area to zero" */
                if(size > 8192 || !(size % 4096)) {
                    storage::fill(p, 0, size);
                }
            }
            /* On sucess we will put a zero (we can also indicate error by setting non-zerro) */
            frame->r15 = (p == nullptr) ? 1 : 0;
            kpanic("TODO: VRU getmain\r\n");
        }
        /* FREEMAIN
         * R0 has the length
         * R1 has the address of the area to be freed
         * R15 status
         */
        else {
            /** @todo Use length for ECC checks */
            storage::free((void *)frame->r1);

            /* On sucess we will put a zero (we can also indicate error by setting non-zerro) */
            frame->r15 = 0;
        }
    } break;
#endif

struct pe_header {
    uint8_t name[8]; // Name of this module
    uint8_t ttr_member[3]; // TTR of the first block of a named member
    uint8_t indicator; // Indicator byte
    // Start of the variable lenght user data field
    uint8_t ttr_text[3]; // TTR of first block of text
    uint8_t zero; // Zero
    uint8_t ttr_note[3]; // TTR of note list or scatter/translation table
    uint8_t n_note_entries; // Number of entries in note list
    uint16_t attr; // 2-byte attribute field
    int8_t virt_store_req[3]; // Total contigous virtual storage requirements for this module
    int16_t first_text_len; // Lenght of the first block of text
    uint8_t entry_addr[3]; // Entry point address
    // Linker-editor assigned origin of the first block of text (when bit 0 is off); otherwise it's the flag bytes
    union {
        uint8_t text_origin[3];
        uint8_t flags[3];
    }ft_set;
    union {
        uint8_t n_rlds; // Number of RLD/CTL records after the texts
        uint8_t flags2; // 4th byte of FTBO (the flags bytes)
    }ft2_set;
    int16_t n_scatter_bytes; // Number of scatter bytes
    int16_t n_trans_bytes; // Number of bytes in translation table
    int16_t ctrl_esdid; //  ESDID of control section of the first text block
    int16_t entry_esdid; // ESDID of control section containing the entry point
    // Below here only applies to load modules with alias
    /// @todo Add alias support
};

/*
int pe_load(void *buffer, size_t n)
{
    struct pe_header *hdr;
    return 0;
}
*/
#define VERSION_STRING "v1.0"

int main(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "/VERSION")) {
            printf("UDOS<->MVS " VERSION_STRING "\r\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Unknown option %s\r\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
