LIBGL_SRC := $(shell find . -type f \( -iname '*.cxx' \) )
LIBGL_OBJS := $(patsubst ./%.cxx,%.o,$(LIBGL_SRC))

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: libgl.so libgl.a
clean:
	$(RM) *.so *.a *.o *.d

.PHONY: all build clean

# -----------------------------------------------------------------------------
# Library for Input/Output
#
# -----------------------------------------------------------------------------
%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(SHARED_LIB_CFLAGS) -I. -c $< -o $@

%.o: %.S
	$(CXX) $(CXXFLAGS) $(SHARED_LIB_CFLAGS) -I. -c $< -o $@

libgl.so: $(LIBGL_OBJS)
	$(CXX) $(LDFLAGS) $(SHARED_LIB_LDFLAGS) -static-libgcc -lgcc $^ -o $@

libgl.a: $(LIBGL_OBJS)
	$(AR) rcs $@ $^

-include $(LIBGL_SRC:.cxx=.d)
