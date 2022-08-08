#ifndef __PS2_HXX__
#define __PS2_HXX__ 1

#include <types.hxx>

namespace ps2 {
	constexpr auto data_port = 0x60;
	constexpr auto status_reg = 0x64;
	constexpr auto command_reg = 0x64;

	/// @brief Controller commands
	namespace cmd {
		enum cmd {
			READ_BYTE = 0x20, // Base, 0x20 thru 0x3F for 0x00 and 0x1F bytes
			WRITE_BYTE = 0x60,
			DISABLE_SECOND_PORT = 0xA7,
			ENABLE_SECOND_PORT = 0xA8,
			TEST_SECOND_PORT = 0xA9,
			TEST_CONTROLLER = 0xAA, // 0x55 = passed, 0xFC = failed
			TEST_FIRST_PORT = 0xAB,
			DIAGNOSTIC_DUMP = 0xAC,
			DISABLE_FIRST_PORT = 0xAD,
			ENABLE_FIRST_PORT = 0xAE,
			READ_CONTROLLER_INPUT_PORT = 0xC0,
			COPY_0_3_TO_4_7 = 0xC1, // Copy bits 0 to 3 of input port to status bits 4 to 7
			COPY_4_7_TO_4_7 = 0xC2, // Copy bits 4 to 7 of input port to status bits 4 to 7
			READ_CONTROLLER_OUTPUT_PORT = 0xD0,
			WRITE_CONTROLLER_PORT = 0xD1, // Check that buffer empty first
			WRITE_FIRST_OUTPUT_PORT = 0xD2,
			WRITE_SECOND_OUTPUT_PORT = 0xD3,
			WRITE_SECOND_INPUT_PORT = 0xD4,
		};
	}

	/// @brief Keyboard device commands
	namespace kb_cmd {
		enum kb_cmd {
			SET_LEDS = 0xED,
			ECHO = 0xEE,
			SCAN_CODE = 0xF0,
			IDENTIFY_KEYBOARD = 0xF2,
			TYPEMATIC_RATE_DELAY = 0xF3,
			ENABLE_SCANNING = 0xF4,
			DISABLE_SCANNING = 0xF5,
			SET_DEFAULT_PARAM = 0xF6,
			SET_ALL_TO_TM_AR = 0xF7, // Typematic, autorepeat only
			SET_ALL_TO_MK_RL = 0xF8, // Make, release
			SET_ALL_TO_MK = 0xF9, // Make only
			SET_ALL_TO_TM_AR_MK_RL = 0xFA, // Typematic, autorepeat, make, release
			SET_SKEY_TM_AR = 0xFB, // Set specific key to typematic, autorepeat only
			SET_SKEY_MK_RL = 0xFC, // Set specific key to make/release
			SET_SKEY_MK = 0xFD, // Set specific key to make only
			RESEND = 0xFE, // Resend last byte
			RESET_AND_SELFTEST = 0xFF, // Reset and start self test
		};
	}

	/// @brief Response bytes
	namespace rsp {
		enum rsp {
			KEY_DETECT_ERROR1 = 0x00,
			SELF_TEST_PASSED = 0xAA,
			ECHO = 0xEE,
			ACK = 0xFA,
			SELFTEST_FAIL1 = 0xFC,
			SELFTEST_FAIL2 = 0xFD,
			RESEND = 0xFE,
			KEY_DETECT_ERROR2 = 0xFF,
		};
	}
}

#endif
