CC = x86_64-w64-mingw32-gcc

SRCS += main.c

CCFLAGS += -O3
CCFLAGS += -DNDEBUG=1

LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections -lm

BUILDDIR = ../build

# library to be generated
MAIN_BIN = downsample_algo_standalone.exe

$(BUILDDIR)/$(MAIN_BIN) : $(SRCS)
	$(CC) $(CCFLAGS) -o $@ $(SRCS) $(LDFLAGS)

all: $(MAIN_BIN)

info:
	echo $(TARGET_TOOLCHAIN_ROOT)
	echo $(TARGET_TOOLCHAIN_PREFIX)

clean:
	rm -f $(MAIN_BIN)
