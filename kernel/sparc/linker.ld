ENTRY(start);
MEMORY {
	ram (rwx) : ORIGIN = 0x100, LENGTH = 4096M
}

#include "csect.ld"
