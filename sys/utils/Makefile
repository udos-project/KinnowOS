UTILS_SRC := $(shell find . -type f -iname '*.cxx' )
UTILS_OBJS := $(patsubst ./%.cxx,%.exe,$(UTILS_SRC))

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: $(UTILS_OBJS)
clean:
	$(RM) *.so *.a *.o *.d *.exe

.PHONY: all build clean

# -----------------------------------------------------------------------------
# Utilities
#
# -----------------------------------------------------------------------------
%.exe: %.cxx
	$(CXX) $(CXXFLAGS) $(PRG_CFLAGS) -I. $(LDFLAGS) $(PRG_LDFLAGS) $< -lio -o $@

-include $(UTILS_SRC:.cxx=.d)
