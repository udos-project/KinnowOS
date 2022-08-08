#pragma once

namespace virtual_machine {
	struct sie_state {
		/* 0 */
		uint8_t intervent_req;
		uint8_t state_ctrl;
		uint8_t resv1;
		uint8_t mode_ctrl;
		uint32_t prefix;
		uint16_t ms_origin;
		uint16_t ms_extent;
		uint32_t resv2;
		/* 16 */
		uint32_t r14;
		uint32_t r15;
		uint64_t psw;
		/* 32 */
		uint32_t resv3;
		uint32_t residue;
		uint64_t cpu_timer;
		/* 48 */
		uint64_t clock_comp;
		uint64_t epoch_diff;
		/* 64 */
		uint32_t svc_ctrl;
		uint16_t lctl_ctrl;
		uint16_t resv4;
		uint64_t intercept_ctrl;
		/* 80 */
		uint8_t intercept_code;
		uint8_t intercept_status;
		uint16_t host_cpu_addr;
		uint16_t resv5;
		/* Instruction parameters */
		uint16_t ipa;
		uint32_t ipb;
		uint32_t ipc;
		/* 96 */
		uint32_t rcp_origin; // RCP Area origin
		uint32_t sysctrl_origin; // System control area origin
		uint64_t resv6;
		/* 112 */
		uint16_t tch_ctrl;
		uint16_t resv7;
		uint32_t resv8;
		uint64_t resv9;
		/* 128 */
		uint64_t cr[16];
		/* 192 */
		uint64_t iparm[4]; // Interruption parameters
		/* 224 */
		uint64_t resv10[4];
	};
};
