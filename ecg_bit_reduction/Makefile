CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++

SRCS += ../shared/data_processing.c
SRCS += ../shared/csv_writers.c
SRCS += ecg_bit_reduction.c
SRCS += main.c

CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -DTF_LITE_MCU_DEBUG_LOG -funsigned-char -fomit-frame-pointer -MD -D__FPU_PRESENT=1 -I. -I../shared

CCFLAGS += -O3
CXXFLAGS += -DNDEBUG=1
CCFLAGS += -DNDEBUG=1

LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections -lm

BUILDDIR = ../build

# library to be generated
MAIN_BIN = ecg_bit_reduction.exe

$(BUILDDIR)/$(MAIN_BIN) : $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDFLAGS)

all: $(MAIN_BIN)

info:
	echo $(TARGET_TOOLCHAIN_ROOT)
	echo $(TARGET_TOOLCHAIN_PREFIX)

clean:
	rm -f $(BUILDDIR)/$(MAIN_BIN)
