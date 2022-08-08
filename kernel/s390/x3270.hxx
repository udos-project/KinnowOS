#pragma once

#include <types.hxx>
#include <s390/asm.hxx>
#include <s390/css.hxx>
#include <storage.hxx>

namespace x3270 {
	namespace cmd {
		enum cmd {
			ENABLE = 0x27,
			SELECT = 0x0B,
			SELECT_WRITE = 0x4B,
			WRITE_NOCR = css::cmd::WRITE,
			WRITE_CR = 0x09,
			READ_BUFFER = css::cmd::READ,
			READ_MOD = 0x06,
			READ_MOD_ALL = 0x0E,
			NOP = css::cmd::CONTROL,
			WSF = 0x11,
		};
	};

	struct node_data {
		css::device::id id;
	};

	void init(css::device::id id);
}
