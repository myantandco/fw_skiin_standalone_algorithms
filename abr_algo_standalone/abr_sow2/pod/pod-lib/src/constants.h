#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <cstdint>

constexpr int kOutputSize = 1;
constexpr int kStateInputSize = 18;

// UART params
constexpr int kMaximumBufferSize = 512;
constexpr int kHostInputSize = 3;

// Model params
constexpr int kNumInputs = 2;
constexpr int kNumOutputs = 2;
constexpr int kModelInputSize = 3;
constexpr int kTensorArenaSize = 5000;
constexpr int kArrayStateSize[1] = { 18 };

#endif  // __CONSTANTS_H__
