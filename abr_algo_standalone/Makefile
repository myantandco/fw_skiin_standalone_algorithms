CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++

SRCS := abr/src/custom_chest.cc
SRCS += abr/src/custom_waist.cc
SRCS += abr/src/custom_kernels.cc
SRCS += abr/src/custom_cmsis_kernels.cc
SRCS += abr/src/model.cpp
SRCS += myant/data_processing.c
SRCS += myant/abr_postprocess.c
SRCS += myant/abr_preprocess.c
SRCS += myant/ecg_algo.cpp
SRCS += main.c

CMSIS_SRCS := abr/cmsis/CMSIS/NN/Source/BasicMathFunctions/arm_elementwise_add_s8.c
CMSIS_SRCS += abr/cmsis/CMSIS/NN/Source/FullyConnectedFunctions/arm_fully_connected_s8.c
CMSIS_SRCS += abr/cmsis/CMSIS/NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8.c
CMSIS_SRCS += abr/cmsis/CMSIS/NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s8.c

SRCS += $(CMSIS_SRCS)

CXXFLAGS += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -DTF_LITE_MCU_DEBUG_LOG -funsigned-char -fomit-frame-pointer -MD -D__FPU_PRESENT=1 -I. -I./abr -I./abr/src -I./abr/gemmlowp -I./abr/cmsis -I./myant -I./abr/cmsis/CMSIS/NN/Include

CCFLAGS += -O3
CXXFLAGS += -DNDEBUG=1
CCFLAGS += -DNDEBUG=1

LDFLAGS += -Wl,--fatal-warnings -Wl,--gc-sections -lm

BUILDDIR = ../build

# library to be generated
MAIN_BIN = abr_algorithm_standalone.exe

$(BUILDDIR)/$(MAIN_BIN) : $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDFLAGS)

all: $(MAIN_BIN)

info:
	echo $(TARGET_TOOLCHAIN_ROOT)
	echo $(TARGET_TOOLCHAIN_PREFIX)

clean:
	rm -f $(MAIN_BIN)
