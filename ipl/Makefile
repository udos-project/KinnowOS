# -----------------------------------------------------------------------------
# Initial Program Loader flags
#
# -----------------------------------------------------------------------------
ifeq ($(DEBUG_IPL), true)
IPL_CXXFLAGS += -g -DDEBUG
endif
ifeq ($(CFG_TARGET), s390)
IPL_CXXFLAGS += -mwarn-dynamicstack -mno-backchain -mpacked-stack \
	-msmall-exec -mstack-size=0 -fno-stack-protector -fno-builtin \
	-fno-builtin-functions
endif

IPL_LDFLAGS := -Wl,-static,--no-dynamic-linker,-ztext -fno-pie -fno-pic \
	-static -Tlinker.ld $(ARCH_LDFLAGS)

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: ipl.pe
clean:
	$(RM) *.pe ipl *.bin

.PHONY: all build clean

# -----------------------------------------------------------------------------
# Initial Program Loader (IPL)
#
# -----------------------------------------------------------------------------
ipl: ipl.S
	$(CXX) $(CXXFLAGS) $(IPL_CXXFLAGS) $(LDFLAGS) $(IPL_LDFLAGS) -fwhole-program $^ -o $@

ipl.bin: ipl
	$(OBJCOPY) -O binary $< $@

ipl.pe: ipl.bin
	../tools/bin2pe.exe $< $@

