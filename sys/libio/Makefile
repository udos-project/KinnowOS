LIBIO_SRC := $(shell find . -type f \( -iname '*.cxx' -or -iname '*.S' \) )
LIBIO_OBJS := $(patsubst ./%.cxx,%.o,$(patsubst ./%.S,%.o,$(LIBIO_SRC)))

# -----------------------------------------------------------------------------
# Build process
#
# -----------------------------------------------------------------------------
all: build
build: libio.so libio.a
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

libio.so: $(LIBIO_OBJS)
	$(CXX) $(LDFLAGS) $(SHARED_LIB_LDFLAGS) -static-libgcc -lgcc $^ -o $@

libio.a: $(LIBIO_OBJS)
	$(AR) rcs $@ $^

-include $(LIBIO_OBJS:.o=.d)
