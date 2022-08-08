JDA_SRC := $(shell find . -type f \( -iname '*.cxx' -or -iname '*.S' \) )
JDA_OBJS := $(patsubst ./%.cxx,%.o,$(patsubst ./%.S,%.o,$(JDA_SRC)))

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: jda.exe
clean:
	$(RM) *.so *.a *.o *.d *.exe

.PHONY: all build clean

# -----------------------------------------------------------------------------
# JDA
#
# -----------------------------------------------------------------------------
%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(PRG_CFLAGS) -I. -c $< -o $@

%.o: %.S
	$(CXX) $(CXXFLAGS) $(PRG_CFLAGS) -I. -c $< -o $@

jda.exe: $(JDA_OBJS) ../libio/libio.a
	$(CXX) $(LDFLAGS) $(PRG_LDFLAGS) -static $^ -o $@

-include $(JDA_SRC:.cxx=.d)
