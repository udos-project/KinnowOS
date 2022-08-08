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

%.exe: %.cxx
	g++ -std=c++17 -g -O2 -Wall -Wextra $< -o $@ -pthread

-include $(UTILS_SRC:.cxx=.d)
