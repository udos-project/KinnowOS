#ifndef ABI_BITS_H
#define ABI_BITS_H

#ifndef __LIBC_ABI
#	include <types.hxx>
#else
#	include <stddef.h>
#	include <stdint.h>
#endif

#define SVC_GET_PDB 0
#define SVC_SET_PDB 1
typedef uintptr_t (*svc_handle_t)(uintptr_t arg1);
struct program_data_block {
	char *pdb_cmdline; /* Full command line, terminating 0 tells length */
#define PDB_ENCODING_ASCII 1 /* AMERICAN STANDARD CHARACTERS */
#define PDB_ENCODING_EBCDIC_1047 2 /* EBCIDC-1047 (INTERNATIONAL) */
#define PDB_ENCODING_EBCDIC_930 3 /* EBCDIC-930 (JAPAN) */
#define PDB_ENCODING_UTF8 4 /* UTF-8 */
#define PDB_ENCODING_CP437 5 /* DOS CP437 (INTERNATIONAL) */
#define PDB_ENCODING_CP866 6 /* CP866 (CYRILIC) */
#define PDB_ENCODING_CP850 7 /* DOS CP850 (WESTERN EUROPEAN) */
	int pdb_encoding; /* Encoding of the application */
	int pdb_opt; /* Options for execution */
	int pdb_errcode; /* Errror code from request */
	size_t pdb_used_bytes; /* Used bytes by the job */
	size_t pdb_free_bytes; /* Free bytes on the job */
	size_t pdb_total_bytes; /* Total bytes that can be allocated to the job */
	svc_handle_t pdb_handles[0x100]; /* Custom SVC handles */
} __attribute__((aligned(4)));

#define SVC_SCHED_YIELD 2
#define SVC_ABEND 3
#define SVC_READ_STDIN 4
#define SVC_PRINT_DEBUG 5
#define SVC_GET_STORAGE 6
#define SVC_DROP_STORAGE 7
#define SVC_RESIZE_STORAGE 8
#define SVC_ALLOC_REAL_STORAGE 9

#define SVC_VFS_OPEN 10
#define SVC_VFS_CLOSE 11
#define SVC_VFS_WRITE 12
#define SVC_VFS_READ 13
#define SVC_VFS_FLUSH 14
#define SVC_VFS_IOCTL 15
#define SVC_VFS_ADD_NODE 16 /* Add a child node to a node */
#define SVC_VFS_REMOVE_NODE 17
#define SVC_VFS_NODE_COUNT 18 /* Obtain the count of child nodes */
#define SVC_VFS_FILLDIR 19 /* Get the list of directories */

#define SVC_CSS_GET_DEV 20
#define SVC_CSS_REQ_CREAT 21
#define SVC_CSS_REQ_SEND 22
#define SVC_CSS_REQ_DELETE 23
#define SVC_CSS_REQ_XCHG_FLAGS 24
#define SVC_CSS_REQ_AWAIT 25 /* Obtain completion status of REQ */
#define SVC_CSS_REQ_GET_STATUS 26 /* Get status of REQ */
#define SVC_CSS_DEVICE_QUERY 27 /* Query a CSS device property */

/* SVC_CSS_DEVICE_QUERY queries */
#define SVC_CSS_DEVICE_QUERY_SCSW_COUNT 0

#define SVC_CSS_REQ_SET_CCW 28
struct css_req_set_ccw_data {
	int ccw_num; /* The CCW to modify */
	int cmd; /* Command to execute */
	void *addr; /* Address of the ccw data */
	int addr_ccw_num; /* -1 for noop, otherwise set req->addr to one of the ccws */
	size_t size; /* Size of the data */
	int flags; /* Flags for the CCW */
};

#define SVC_EXEC_AT 29 /* Execute a new task at */
#define SVC_THREAD_AT 30 /* Execute a new thread at */

/** @todo Implement futexes */
#define SVC_FUTEX_WAIT 31
#define SVC_FUTEX_WAKE 32

#endif
