# MAKE_ARM = 1

ifdef MAKE_ARM
  TARGET_TOOLCHAIN_ROOT := /opt/gcc_embedded/bin/
  TARGET_TOOLCHAIN_PREFIX := arm-none-eabi-
else
  TARGET_TOOLCHAIN_ROOT := /usr/bin/
  TARGET_TOOLCHAIN_PREFIX := ''
endif

CMSIS_DIR := cmsis/CMSIS/NN/Source

# These are microcontroller-specific rules for converting the ELF output
# of the linker into a binary image that can be loaded directly.
CXX             := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)g++'
CC              := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)gcc'
AS              := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)as'
AR              := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)ar'
LD              := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)ld'
NM              := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)nm'
OBJDUMP         := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)objdump'
OBJCOPY         := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)objcopy'
SIZE            := '$(TARGET_TOOLCHAIN_ROOT)$(TARGET_TOOLCHAIN_PREFIX)size'

RM = rm -f
ARFLAGS := -csr
SRCS := src/custom_chest.cc src/custom_waist.cc
SRCS += src/custom_kernels.cc

ifdef MAKE_ARM
  SRCS += src/custom_cmsis_kernels.cc
  # DEFINES := -DCMSIS_ADD
  # DEFINES := -DCMSIS_FC
  DEFINES := -DCMSIS_ADD -DCMSIS_FC

  # NOTE: CMSIS_FC_EXTRA is used for ops that are slower with CMSIS,
  # as last checked for the SOW2 chest model.
  # DEFINES := -DCMSIS_ADD -DCMSIS_FC -DCMSIS_FC_EXTRA

  # CMSIS source additions
  SRCS += $(CMSIS_DIR)/BasicMathFunctions/arm_elementwise_add_s8.c
  SRCS += $(CMSIS_DIR)/FullyConnectedFunctions/arm_fully_connected_s8.c
  SRCS += $(CMSIS_DIR)/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8.c
  SRCS += $(CMSIS_DIR)/NNSupportFunctions/arm_nn_vec_mat_mult_t_s8.c
else
  SRCS += src/custom_cmsis_kernels.cc  # just for CMSIS_FillFCParams
endif
SRCS += src/main.cc

OBJS := $(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(SRCS)))

LIBRARY_OBJS := $(filter-out src/main.o, $(OBJS))

ifdef MAKE_ARM
  CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter  -mcpu=cortex-m4 -DTF_LITE_MCU_DEBUG_LOG -mthumb -mfloat-abi=hard -funsigned-char -mlittle-endian -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -DCPU_M4=1 -D__FPU_PRESENT=1 -mfpu=fpv4-sp-d16 -I. -I./src -I./gemmlowp $(DEFINES)
  CCFLAGS += -std=c11 -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter  -mcpu=cortex-m4 -DTF_LITE_MCU_DEBUG_LOG -mthumb -mfloat-abi=hard -funsigned-char -mlittle-endian -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -DCPU_M4=1 -D__FPU_PRESENT=1 -mfpu=fpv4-sp-d16 -I. -I./third_party/gemmlowp -I./third_party/flatbuffers/include -I./third_party/ruy
else
  CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DTF_LITE_MCU_DEBUG_LOG -funsigned-char -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer -MD -D__FPU_PRESENT=1 -I. -I./src -I./gemmlowp
endif

CXXFLAGS += -O3
CCFLAGS += -O3
CXXFLAGS += -DNDEBUG=1
CCFLAGS += -DNDEBUG=1

# CXXFLAGS += -DTF_LITE_STRIP_ERROR_STRINGS=0

# CXXFLAGS += -DTF_LITE_STRIP_ERROR_STRINGS=1 -DNDEBUG=1

ifdef MAKE_ARM
  # CMSIS flag and includes
  CXXFLAGS += -DARM_MATH_DSP -I./cmsis/CMSIS/Core_A/Include
  CCFLAGS += -DARM_MATH_DSP -I./cmsis/CMSIS/Core_A/Include
endif

LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections -lm

# library to be generated
MAIN_BIN = custom_bin
ifdef MAKE_ARM
  MICROLITE_LIB = libabr-myant-arm-rpeak.a
else
  MICROLITE_LIB = libabr-myant-cpu-rpeak.so
endif

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

$(MAIN_BIN) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)


# Creates a tensorflow-litemicro.a which excludes any example code.
$(MICROLITE_LIB): $(LIBRARY_OBJS)
	$(AR) $(ARFLAGS) $(MICROLITE_LIB) $(LIBRARY_OBJS)

all: $(MAIN_BIN) $(MICROLITE_LIB)

lib: $(MICROLITE_LIB)

info:
	echo $(TARGET_TOOLCHAIN_ROOT)
	echo $(TARGET_TOOLCHAIN_PREFIX)

clean:
	-$(RM) $(OBJS)
	-$(RM) $(MAIN_BIN)
	-$(RM) ${MICROLITE_LIB}
