# Based on TFLM hello_world example Makefile and Nordic SDK UART example Makefile

# SDK_ROOT := <path/to/SDK>/nRF5SDK1702d674dde/nRF5_SDK_17.0.2_d674dde

ifdef MAKE_ARM
  TARGET_TOOLCHAIN_ROOT ?= /opt/gcc_embedded/bin/
  TARGET_TOOLCHAIN_PREFIX := arm-none-eabi-
else
  TARGET_TOOLCHAIN_ROOT := /usr/bin/
  TARGET_TOOLCHAIN_PREFIX := ''
endif

CXX             := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)g++'
CC              := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)gcc'
AS              := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)as'
AR              := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)ar'
LD              := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)ld'
NM              := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)nm'
OBJDUMP         := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)objdump'
OBJCOPY         := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)objcopy'
SIZE            := '$(TARGET_TOOLCHAIN_ROOT)/$(TARGET_TOOLCHAIN_PREFIX)size'

RM = rm -f
ARFLAGS := -csr

ifdef MAKE_ARM
  ifndef SDK_ROOT
    $(error SDK_ROOT is not set.)
  endif

  SRCS := \
    $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
    $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
    $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
    $(SDK_ROOT)/components/boards/boards.c \
    $(SDK_ROOT)/components/libraries/util/app_error.c \
    $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
    $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
    $(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
    $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c \
    $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
    $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
    $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
    $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
    $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
    $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
    $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
    $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
    $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
    $(SDK_ROOT)/components/libraries/uart/retarget.c \
    $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
    $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
    $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
    $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
    $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
    $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
    main.cc

  INCLUDES := \
    -I$(SDK_ROOT)/components \
    -I$(SDK_ROOT)/modules/nrfx/mdk \
    -I$(SDK_ROOT)/components/libraries/fifo \
    -I$(SDK_ROOT)/components/libraries/strerror \
    -I$(SDK_ROOT)/components/toolchain/cmsis/include \
    -I$(SDK_ROOT)/components/libraries/util \
    -I$(SDK_ROOT)/examples/peripheral/uart/pca10056/blank/config \
    -I$(SDK_ROOT)/components/libraries/balloc \
    -I$(SDK_ROOT)/components/libraries/ringbuf \
    -I$(SDK_ROOT)/modules/nrfx/hal \
    -I$(SDK_ROOT)/components/libraries/bsp \
    -I$(SDK_ROOT)/components/libraries/uart \
    -I$(SDK_ROOT)/components/libraries/log \
    -I$(SDK_ROOT)/modules/nrfx \
    -I$(SDK_ROOT)/components/libraries/experimental_section_vars \
    -I$(SDK_ROOT)/integration/nrfx/legacy \
    -I$(SDK_ROOT)/components/libraries/delay \
    -I$(SDK_ROOT)/integration/nrfx \
    -I$(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
    -I$(SDK_ROOT)/components/libraries/atomic \
    -I$(SDK_ROOT)/components/boards \
    -I$(SDK_ROOT)/components/libraries/memobj \
    -I$(SDK_ROOT)/modules/nrfx/drivers/include \
    -I$(SDK_ROOT)/external/fprintf \
    -I$(SDK_ROOT)/components/libraries/log/src

  LINKER_SCRIPT  := \
  $(SDK_ROOT)/examples/peripheral/uart/pca10056/blank/armgcc/uart_gcc_nrf52.ld

  # From TFLM
  CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -O3 -Werror -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DCORTEX_M_GENERIC -DCMSIS_NN -mcpu=cortex-m4 -mfpu=auto -DTF_LITE_MCU_DEBUG_LOG -mthumb -funsigned-char -mlittle-endian -Wno-implicit-fallthrough -Wno-strict-aliasing -Wno-unused-variable -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -DCPU_M4=1 -D__FPU_PRESENT=1 -I.
  CCFLAGS +=  -std=c11 -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -O3 -Werror -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DCORTEX_M_GENERIC -DCMSIS_NN -mcpu=cortex-m4 -mfpu=auto -DTF_LITE_MCU_DEBUG_LOG -mthumb -funsigned-char -mlittle-endian -Wno-implicit-fallthrough -Wno-strict-aliasing -Wno-unused-variable -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -DCPU_M4=1 -D__FPU_PRESENT=1 -I.
  LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections -lm
  CCFLAGS += -Wno-expansion-to-defined

  # From Nordic SDK uart example
  # C flags common to all targets
  CFLAGS += -DARM_MATH_CM4
  CFLAGS += -DBOARD_PCA10056
  CFLAGS += -DBSP_DEFINES_ONLY
  CFLAGS += -DCONFIG_GPIO_AS_PINRESET
  CFLAGS += -DFLOAT_ABI_HARD
  CFLAGS += -DNRF52840_XXAA
  CFLAGS += -mcpu=cortex-m4
  CFLAGS += -mthumb -mabi=aapcs
  CFLAGS += -Wall -Werror
  CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
  # keep every function in a separate section, this allows linker to discard unused ones
  CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
  CFLAGS += -fno-builtin -fshort-enums

  CFLAGS += -DMAKE_ARM

  # Assembler flags common to all targets
  ASMFLAGS += -O3
  ASMFLAGS += -mcpu=cortex-m4
  ASMFLAGS += -mthumb -mabi=aapcs
  ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
  ASMFLAGS += -DBOARD_PCA10056
  ASMFLAGS += -DBSP_DEFINES_ONLY
  ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
  ASMFLAGS += -DFLOAT_ABI_HARD
  ASMFLAGS += -DNRF52840_XXAA

  # Linker flags
  LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
  LDFLAGS += -mcpu=cortex-m4
  LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
  # let linker dump unused sections
  LDFLAGS += -Wl,--gc-sections
  # use newlib in nano version
  LDFLAGS += --specs=nano.specs

  CFLAGS += -D__HEAP_SIZE=8192
  CFLAGS += -D__STACK_SIZE=8192
  ASMFLAGS += -D__HEAP_SIZE=8192
  ASMFLAGS += -D__STACK_SIZE=8192

  LIB_FILES += -lc -lnosys -lm -labr-myant-arm-rpeak

  # combine flags
  CCFLAGS += $(CFLAGS)
  CXXFLAGS += $(CFLAGS)

  # Add float prints
  LDFLAGS += -u _printf_float

else
  SRCS := main.cc
  INCLUDES :=

  CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DTF_LITE_MCU_DEBUG_LOG -funsigned-char -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -D__FPU_PRESENT=1 -I. -I./src -I./gemmlowp

  CXXFLAGS += -O3
  CCFLAGS += -O3
  CXXFLAGS += -DNDEBUG=1
  CCFLAGS += -DNDEBUG=1

  LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections
  LIB_FILES += -L. -lc -lm -labr-myant-cpu-rpeak

endif

OBJS := \
$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SRCS))))

ifdef MAKE_ARM
  MAIN_BIN = abr-myant-arm-rpeak-demo
else
  MAIN_BIN = abr-myant-cpu-rpeak-demo
endif

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.S
	$(CC) -x assembler-with-cpp $(ASMFLAGS) $(INCLUDES) -c $< -o $@

abr-myant-arm-rpeak-demo.hex : abr-myant-arm-rpeak-demo
	$(OBJCOPY) -O ihex $< $@

abr-myant-cpu-rpeak-demo : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB_FILES) $(LDFLAGS)

abr-myant-arm-rpeak-demo : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB_FILES) $(LDFLAGS)

all: $(MAIN_BIN)

clean:
	-$(RM) $(OBJS)
	-$(RM) abr-myant-arm-rpeak-demo*
	-$(RM) abr-myant-cpu-rpeak-demo*

new: clean abr-myant-arm-rpeak-demo.hex

print_srcs:
	@echo $(SRCS)
print_objs:
	@echo $(OBJS)
