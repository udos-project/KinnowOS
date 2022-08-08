# -----------------------------------------------------------------------------
# Architecture dependant code
#
# -----------------------------------------------------------------------------
# s390
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), s390)
ARCH_KERNEL_SRC := $(shell find s390 -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif
# -----------------------------------------------------------------------------
# OpenRISC
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), or1k)
ARCH_KERNEL_SRC := $(shell find or1k -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif
# -----------------------------------------------------------------------------
# SPARC
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), sparc)
ARCH_KERNEL_SRC := $(shell find sparc -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif
# -----------------------------------------------------------------------------
# RISC-V
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), riscv)
ARCH_KERNEL_SRC := $(shell find riscv -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif
# -----------------------------------------------------------------------------
# Itanium 64
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), ia64)
ARCH_KERNEL_SRC := $(shell find ia64 -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif
# -----------------------------------------------------------------------------
# x86
# -----------------------------------------------------------------------------
ifeq ($(CFG_TARGET), x86)
ARCH_KERNEL_SRC := $(shell find x86 -maxdepth 1 -type f \( -iname '*.cxx' \) )
endif

# -----------------------------------------------------------------------------
# Kernel flags
#
# -----------------------------------------------------------------------------
KERNEL_OPTIMIZE_CXXFLAGS := -Os -ffast-math \
	-flive-range-shrinkage -free -fsched-pressure -fno-math-errno \
	-fno-signed-zeros -fno-trapping-math -funroll-loops -fgcse-sm -fgcse-las \
	-fomit-frame-pointer
ifeq ($(CFG_TARGET), s390)
KERNEL_OPTIMIZE_CXXFLAGS += -mmvcle -mno-backchain -mpacked-stack
endif

KERNEL_CXXFLAGS := $(KERNEL_OPTIMIZE_CXXFLAGS) -ffreestanding \
	-fno-stack-protector -fno-pic -fno-rtti -fno-exceptions \
	-fno-use-cxa-get-exception-ptr -fstrict-enums -fno-extern-tls-init \
	-fno-use-cxa-atexit -fno-stack-protector -I.
ifeq ($(CFG_TARGET), s390)
KERNEL_CXXFLAGS += -mwarn-dynamicstack -mno-vx -msoft-float
else
ifeq ($(CFG_TARGET), or1k)
KERNEL_CXXFLAGS += -mhard-div -mhard-mul -mhard-float -munordered-float \
	-mcmov -mror -mrori -msext -msfimm -mshftimm -mcmodel=small
else
ifeq ($(CFG_TARGET), riscv)
KERNEL_CXXFLAGS += -mcmodel=medany
else
endif
endif
endif

KERNEL_LDFLAGS := -Wl,-static,--no-dynamic-linker,-ztext \
	-fno-pie -static -fwhole-program
ifeq ($(CFG_TARGET), s390)
KERNEL_LDFLAGS += -T s390/linker.ld
else
ifeq ($(CFG_TARGET), or1k)
KERNEL_LDFLAGS += -T or1k/linker.ld
else
ifeq ($(CFG_TARGET), sparc)
KERNEL_LDFLAGS += -T sparc/linker.ld
else
ifeq ($(CFG_TARGET), riscv)
KERNEL_LDFLAGS += -T riscv/linker.ld -mcmodel=medany
else
ifeq ($(CFG_TARGET), ia64)
KERNEL_LDFLAGS += -T ia64/linker.ld
else
ifeq ($(CFG_TARGET), x86)
KERNEL_LDFLAGS += -T x86/linker.ld
endif
endif
endif
endif
endif
endif

KERNEL_SRC := $(sort $(ARCH_KERNEL_SRC) $(subst ./,,$(shell find . -maxdepth 1 -type f \( -iname '*.cxx' \) )))
KERNEL_OBJS := $(patsubst %.cxx,%.o,$(KERNEL_SRC))
ifeq ($(CFG_KERNEL_DEBUG), true)
KERNEL_CXXFLAGS += -DDEBUG=1 -g
endif
ifeq ($(CFG_KERNEL_UBSAN), true)
KERNEL_CXXFLAGS += -fsanitize=undefined
endif

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: arch kernel.bin

clean:
	$(RM) *.d *.o *.bin kernel

.PHONY: all build clean

# -----------------------------------------------------------------------------
# Kernel
#
# -----------------------------------------------------------------------------
%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(KERNEL_CXXFLAGS) -c $< -o $@

kernel: $(KERNEL_OBJS)
	$(CXX) $(LDFLAGS) $(KERNEL_LDFLAGS) $^ -o $@ -static-libgcc -lgcc

kernel.bin: kernel
	$(OBJCOPY) -O binary $< $@

arch:
	$(RM) $@
	ln -s $(CFG_TARGET) $@

-include $(KERNEL_SRC:.cxx=.d)
