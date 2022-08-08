# -----------------------------------------------------------------------------
# Per-used dependant variables. Set these to your current environment
# accordingly.
#
# -----------------------------------------------------------------------------
# Options
# s390		s360		System/360
# s390		s370		System/370
# s390		s370-xa		System/370-XA
# s390		s380		System/380
# s390		s390		System/390
# s390		zarch		z/Arch
# s390      zarch-vx    z/Arch (with vectors)
# s390		default		z/Arch
# or1k		default		Default
# ia64		ia64		Default
# sparc		sparc32		SPARC 32-bits
# sparc		sparc64		SPARC 64-bits
# sparc		default		SPARC 64-bits
# riscv		riscv32		RISC-V 32-bits
# riscv		riscv64		RISC-V 64-bits
# riscv		default		RISC-V 64-bits
# x86		x86			x86 32-bits
# x86		x86_64		x86 64-bits
# x86		default		x86 64-bits
# -----------------------------------------------------------------------------

# Target to use (native = use host's)
CFG_TARGET ?= native
# Machine to use (subtarget)
CFG_MACHINE ?= default
# Software float
CFG_SW_FLOAT ?= false
# Enable fat-link-time-optimization
CFG_FLTO ?= false
# Undefined address sanitizer (only supported on clang/gcc)
CFG_KERNEL_UBSAN ?= false
# Kernel debug mode
CFG_KERNEL_DEBUG ?= false
# Debugging mode enabled for programs
CFG_PGM_DEBUG ?= false
# If we should dump symbols
CFG_DUMP_SYMS ?= false
# Whetever to build programs or not
CFG_DISTRO_BUILD ?= true
# Encoding to use
# Supported encodings: IBM-1047, ASCII, UTF8
CFG_ENCODING ?= native

# Native target means autodetection
ifeq ($(CFG_TARGET), native)
CFG_TARGET := $(shell uname -m)
# TODO: Add support for more unames
ifeq ($(CFG_TARGET), amd64)
CFG_TARGET := x86
CFG_MACHINE := x86_64
else
ifeq ($(CFG_TARGET), i686)
CFG_TARGET := x86
CFG_MACHINE := $(CFG_TARGET)
else
ifeq ($(CFG_TARGET), s390x)
CFG_TARGET := s390
CFG_MACHINE := zarch
endif
endif
endif
endif

export CFG_TARGET
export CFG_MACHINE
export CFG_SW_FLOAT
export CFG_FLTO
export CFG_KERNEL_DEBUG
export CFG_KERNEL_UBSAN
export CFG_PGM_DEBUG
export CFG_DUMP_SYMS
export CFG_DISTRO_BUILD

# Detect current OS
ifeq ($(OS_NAME), FreeDOS) # FreeDOS
OS := freedos
else
ifeq ($(OS), Windows_NT) # Winodws NT
OS := windows
else
ifeq ($(shell uname), Linux) # Linux
OS := linux
endif
endif
endif

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------
ifeq ($(OS), linux)
ROOTDIR := $(shell pwd)
else
ROOTDIR := $(shell cd)
endif
OBJDIR := out

# -----------------------------------------------------------------------------
# Architecture dependant code
#
# -----------------------------------------------------------------------------
# s390
ifeq ($(CFG_TARGET), s390)
TARGET_SUPPORT_SHARED := true
TARGET_SUPPORT_HW_FLOAT := true
CROSS_BIN_PREFIX ?= s390x-ibm-linux-
ARCH_CFLAGS := -DTARGET_S390 -DM_S360=360 -DM_S370=370 -DM_S370_XA=375 \
	-DM_S380=380 -DM_S390=390 -DM_ZARCH=400 -DM_ZARCH_VX=401 -mno-htm
ARCH_LDFLAGS := -z max-page-size=0x1000 -mzarch -m64 -march=z15
# -----------------------------------------------------------------------------
# z/Arch
ifeq ($(CFG_MACHINE), zarch)
ARCH_CFLAGS += -DMACHINE=M_ZARCH -mzarch -m64 -march=z15
else
# s/390
ifeq ($(CFG_MACHINE), s390)
ARCH_CFLAGS += -DMACHINE=M_S390 -mesa -m31
else
# s/370
ifeq ($(CFG_MACHINE), s370)
ARCH_CFLAGS += -DMACHINE=M_S370 -mesa -m31
# (default) - z/Arch
else
# z/Arch with vx
ifeq ($(CFG_MACHINE), zarch-vx)
ARCH_CFLAGS += -DMACHINE=M_ZARCH_VX -mzvector -mvx
else
ARCH_CFLAGS += -DMACHINE=M_ZARCH -mno-zvector -mno-vx
endif
ARCH_CFLAGS += -mzarch -m64 -march=z15
endif
endif
endif
# -----------------------------------------------------------------------------
else
# -----------------------------------------------------------------------------
# OpenRISC
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), or1k)
TARGET_SUPPORT_SHARED := true
TARGET_SUPPORT_HW_FLOAT := true
CROSS_BIN_PREFIX ?= or1k-elf-
ARCH_CFLAGS := -DTARGET_OR1K -DM_OR1K32=32 -DM_OR1K64=64 -DM_OR1K128=128
ARCH_CFLAGS += -DMACHINE=M_OR1K32
ARCH_LDFLAGS := -z max-page-size=0x1000
else
# -----------------------------------------------------------------------------
# SPARC
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), sparc)
TARGET_SUPPORT_SHARED := true
TARGET_SUPPORT_HW_FLOAT := true
ARCH_CFLAGS := -DTARGET_SPARC=1 -DM_SPARC32=32 -DM_SPARC64=64
ifeq ($(CFG_MACHINE), sparc32)
CROSS_BIN_PREFIX ?= sparc-elf-
ARCH_CFLAGS += -DMACHINE=M_SPARC32
else
CROSS_BIN_PREFIX ?= sparc64-elf-
ARCH_CFLAGS += -DMACHINE=M_SPARC64
endif
ARCH_LDFLAGS := -z max-page-size=0x1000
else
# -----------------------------------------------------------------------------
# RISC-V
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), riscv)
TARGET_SUPPORT_SHARED := false
TARGET_SUPPORT_HW_FLOAT := false
ARCH_CFLAGS := -DTARGET_RISCV=1 -DM_RISCV32=32 -DM_RISCV64=64
ifeq ($(CFG_MACHINE), riscv32)
CROSS_BIN_PREFIX ?= riscv32-elf-
ARCH_CFLAGS += -DMACHINE=M_RISCV32 -march=rv32imafd -mabi=rv32
ARCH_LDFLAGS := -march=rv32imafd -mabi=rv32
else
CROSS_BIN_PREFIX ?= riscv-elf-
ARCH_CFLAGS += -DMACHINE=M_RISCV64 -march=rv64imafd -mabi=lp64
ARCH_LDFLAGS := -march=rv64imafd -mabi=lp64
endif
ARCH_LDFLAGS += -z max-page-size=0x1000
else
# -----------------------------------------------------------------------------
# Itanium 64
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), ia64)
TARGET_SUPPORT_SHARED := true
TARGET_SUPPORT_HW_FLOAT := true
CROSS_BIN_PREFIX ?= ia64-elf-linux-
ARCH_CFLAGS := -DTARGET_IA64 -DM_IA64=64
ARCH_CFLAGS += -DMACHINE=M_IA64
ARCH_LDFLAGS := -z max-page-size=0x1000
else
# -----------------------------------------------------------------------------
# x86
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), x86)
TARGET_SUPPORT_SHARED := true
TARGET_SUPPORT_HW_FLOAT := true
ARCH_CFLAGS := -DTARGET_X86=1 -DM_X86=32 -DM_X86_64=64
ifeq ($(CFG_MACHINE), x86_64)
CROSS_BIN_PREFIX ?= x86_64-elf-
ARCH_CFLAGS += -DMACHINE=M_X86_64 -mno-red-zone
else
CROSS_BIN_PREFIX ?= i686-elf-
ARCH_CFLAGS += -DMACHINE=M_X86
endif
ARCH_LDFLAGS := -z max-page-size=0x1000
else

endif
endif
endif
endif
endif
endif

#
# Configure qemu
#
QEMU := qemu-system-$(CFG_TARGET)
ifeq ($(CFG_TARGET), x86)
ifeq ($(CFG_MACHINE), x86_64)
QEMU := qemu-system-x86_64
else
QEMU := qemu-system-i386
endif
else
ifeq ($(CFG_TARGET), riscv)
ifeq ($(CFG_MACHINE), riscv32)
QEMU := qemu-system-riscv32
else
QEMU := qemu-system-riscv64
endif
endif
endif

QEMU_DEVICES ?= -device VGA
QEMU_TRACE ?= -d unimp,guest_errors -trace usb* -trace scsi* -trace ehci* \
	-trace virtio* -trace ahci* -trace pci_nvme* -trace pci* -trace vfio*

HERC_PREFIX := $(HOME)/src/hyperion
export HERC_PREFIX
DASDLOAD := $(HERC_PREFIX)/dasdload
export DASDLOAD
HERCULES := $(HERC_PREFIX)/hercules
export HERCULES
X3270 := x3270
export X3270

# Add the hercules utilities to the path
PATH := ${PATH}:${HERC_PREFIX}
export PATH

# -----------------------------------------------------------------------------
# Environment variables
#
# -----------------------------------------------------------------------------
CROSS_PREFIX ?= $(HOME)/opt/cross/bin/$(CROSS_BIN_PREFIX)

CC := $(CROSS_PREFIX)gcc
CFLAGS := $(ARCH_CFLAGS) -pipe -MD -MP -Wfatal-errors -Wall -Wextra -Wshadow \
	-Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-declarations \
	-Wdouble-promotion -Wredundant-decls -Wconversion -I. \
	-fno-builtin -fno-builtin-functions
ifneq ($(CFG_ENCODING), native)
CFLAGS += -fexec-charset=$(CFG_ENCODING)
endif
ifeq ($(CFG_DUMP_SYMS), true)
CFLAGS += -g
endif
ifeq ($(CFG_FLTO), true)
CFLAGS += -flto
endif
export CC
export CFLAGS

CXX := $(CROSS_PREFIX)g++
CXXFLAGS := $(CFLAGS) -std=c++20
export CXX
export CXXFLAGS

AS := $(CROSS_PREFIX)as
ASFLAGS := 
ifeq ($(CFG_DUMP_SYMS), true)
ASFLAGS += -g
endif
export AS
export ASFLAGS

LD := $(CROSS_PREFIX)ld
LDFLAGS := $(ARCH_LDFLAGS) -nostdlib -fno-builtin -fno-builtin-functions
ifeq ($(CFG_DUMP_SYMS), true)
LDFLAGS += -g
endif
ifeq ($(CFG_FLTO), true)
LDFLAGS += -flto
endif
export LD
export LDFLAGS

AR := $(CROSS_PREFIX)ar
export AR

GDB := $(CROSS_PREFIX)gdb
export GDB

OBJCOPY := $(CROSS_PREFIX)objcopy
export OBJCOPY

OBJDUMP := $(CROSS_PREFIX)objdump
export OBJDUMP

STRIP := $(CROSS_PREFIX)strip
export STRIP

COPY := cp
export COPY

# -----------------------------------------------------------------------------
# Programs, utilities and libraries
#
# -----------------------------------------------------------------------------
PRG_CFLAGS := -Os -ffast-math \
	-flive-range-shrinkage -free -fsched-pressure -fno-math-errno \
	-fno-signed-zeros -fno-trapping-math -funroll-loops -fgcse-sm \
	-fgcse-las -I$(ROOTDIR)/sys/libio -I$(ROOTDIR)/sys/libubsan
# s390
ifeq ($(CFG_TARGET), s390)
PRG_CFLAGS += -mmvcle
endif
ifeq ($(CFG_TARGET), riscv)
# RISC-V doesn't support float
else
ifeq ($(CFG_SW_FLOAT), true)
PRG_CFLAGS += -msoft-float
else
PRG_CFLAGS += -mhard-float
endif
endif
ifeq ($(CFG_PGM_DEBUG), true)
PRG_CFLAGS += -DDEBUG=1
endif
PRG_LDFLAGS := -L$(ROOTDIR)/sys/libio -L$(ROOTDIR)/sys/libubsan -T$(ROOTDIR)/sys/pgm.ld -Wl,-gc-sections
export PRG_CFLAGS
export PRG_LDFLAGS

SHARED_LIB_CFLAGS := 
export SHARED_LIB_CFLAGS

SHARED_LIB_LDFLAGS := -L$(ROOTDIR)/sys/lib.ld
ifeq ($(TARGET_SUPPORT_SHARED), true)
SHARED_LIB_LDFLAGS += -shared
else
SHARED_LIB_LDFLAGS += -lgcc -static-libgcc
endif
export SHARED_LIB_LDFLAGS

ifeq ($(CFG_DISTRO_BUILD), true)
CFG_PROGRAM_LIST ?= \
	sys/libio/libio.so \
	sys/jda/jda.exe
endif

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build

build-s390: ipl/ipl.pe kernel/kernel.bin $(OS_LIBS) $(OS_PGM) tools/dir2dasd.exe

build-or1k:
build-ia64:
build-riscv:
build-sparc:
build-x86:

build: kernel/kernel.bin $(CFG_PROGRAM_LIST) build-$(CFG_TARGET)

run-hercules: build
	-$(RM) sysdsk00.cckd
	./tools/dir2dasd.exe -i ipl/ipl.pe -k kernel/kernel.bin -v UDOS00 -p sys/ -o >dasd.dcf
	$(DASDLOAD) dasd.dcf sysdsk00.cckd 3
	$(HERCULES) -f hercules.cnf >log.txt

run-x3270: build
	$(X3270) -model 3279-2 127.0.0.1 3270

run-gdb: build
	$(GDB) 

run-qemu: build
	@$(COPY) kernel/kernel kernel/kernel.o1
	@$(STRIP) --strip-debug --strip-unneeded kernel/kernel.o1
	@$(QEMU) -M virt -S -s -serial stdio -kernel kernel/kernel.o1 $(QEMU_TRACE) $(QEMU_DEVICES) >log.txt

clean:
	$(MAKE) -C kernel $@
	$(MAKE) -C ipl $@
	$(MAKE) -C sys/jda $@
	$(MAKE) -C sys/libio $@
	$(MAKE) -C sys/utils $@
	$(MAKE) -C tools $@

kernel/kernel.bin:
	$(MAKE) -C kernel all

ipl/ipl.pe: tools/bin2pe.exe
	$(MAKE) -C ipl all

sys/jda/jda.exe: sys/libio/libio.a
	$(MAKE) -C sys/jda all

sys/libio/libio.a:
	$(MAKE) -C sys/libio all

sys/libio/libio.so:
	$(MAKE) -C sys/libio all

sys/utils/%.exe:
	$(MAKE) -C sys/utils all

tools/%.exe:
	$(MAKE) -C tools all

.PHONY: all build-s390 build-or1k build-ia64 build-riscv build-x86 build-sparc build run clean run-hercules run-qemu
