#pragma once

#include <types.hxx>

/* Synchronous request */
#define SCCB_FLAG_SYNC 0x80

/* Variable request */
#define SCCB_TYPE_VARIABLE 0x80

enum sclp_sccb_reas {
	SCLP_SCCB_REAS_NONE = 0x00,
	SCLP_SCCB_REAS_NOT_PAGEBOUND = 0x01,
	SCLP_SCCB_REAS_ODD_LENGTH = 0x02,
	SCLP_SCCB_REAS_TOO_SHORT = 0x03,
	SCLP_SCCB_REAS_NOACTION = 0x02,
	SCLP_SCCB_REAS_STANDBY = 0x04,
	SCLP_SCCB_REAS_INVALID_CMD = 0x01,
	SCLP_SCCB_REAS_INVALID_RESPARM = 0x03,
	SCLP_SCCB_REAS_IMPROPER_RES = 0x05,
	SCLP_SCCB_REAS_INVALID_RES = 0x09,
};

enum sclp_sccb_resp {
	/* Data block error */
	SCLP_SCCB_RESP_BLOCK_ERROR = 0x00,
	/* Information returned */
	SCLP_SCCB_RESP_INFO = 0x10,
	/* Command complete */
	SCLP_SCCB_RESP_COMPLETE = 0x20,
	/* Command backed out */
	SCLP_SCCB_RESP_BACKOUT = 0x40,
	/* Command reject */
	SCLP_SCCB_RESP_REJECT = 0xF0,
};

struct sclp_sccb_header {
	uint16_t length;
	uint8_t flag;
	uint8_t reserved;
	uint8_t reas; /* Reason code */
	uint8_t resp; /* Response code */
} PACKED;

struct sclp_sccb {
	struct sclp_sccb_header header;
	uint8_t data[4096 - sizeof(struct sclp_sccb_header)];
} PACKED;

/* For the SYSG console */
static inline int sclp_cmd(unsigned int cmd, struct sclp_sccb *sccb)
{
	int cc = -1;
	asm volatile(
		".insn rre,0xB2200000,%1,%2\r\n"  /* servc %1,%2 */
		"IPM %0\r\n"
		: "=d" (cc)
		: "d" (cmd), "a" (sccb)
		: "cc", "memory");
	return cc >> 28;
}
