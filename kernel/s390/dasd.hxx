#pragma once

#include <types.hxx>
#include <s390/css.hxx>
#include <zdsfs.hxx>

/* DASD CCW commands */
#define DASD_CMD_SEEK 0x07
#define DASD_CMD_WR_LD 0x0D
#define DASD_CMD_LD 0x0E
#define DASD_CMD_SEARCH 0x31

namespace dasd {
	int init(css::device::id id);
}
