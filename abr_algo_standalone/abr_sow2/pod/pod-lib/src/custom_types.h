#ifndef __ABR_CUSTOM_TYPES_H__
#define __ABR_CUSTOM_TYPES_H__

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <initializer_list>
#include <iterator>

#include "cmsis/CMSIS/NN/Include/arm_nn_types.h"

// If we're on a platform without standard IO functions, fall back to a
// non-portable function.
#ifdef TF_LITE_MCU_DEBUG_LOG

#define DEBUG_LOG(x) \
  do {               \
    printf("%s", x); \
  } while (0)

inline void InfiniteLoop() {
  DEBUG_LOG("HALTED\n");
  while (1) {
  }
}

#define TFLITE_ABORT InfiniteLoop();

#else  // TF_LITE_MCU_DEBUG_LOG

#include <cstdio>
#include <cstdlib>

#define DEBUG_LOG(x)            \
  do {                          \
    fprintf(stderr, "%s", (x)); \
  } while (0)

#define TFLITE_ABORT abort()

#endif  // TF_LITE_MCU_DEBUG_LOG

#ifdef NDEBUG
#define TFLITE_ASSERT_FALSE (static_cast<void>(0))
#define TF_LITE_FATAL(msg) (static_cast<void>(0))
#define TF_LITE_ASSERT(x) (static_cast<void>(0))
#define TF_LITE_ASSERT_EQ(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK(condition) (static_cast<void>(0))
#define TFLITE_DCHECK_EQ(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK_NE(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK_GE(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK_GT(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK_LE(x, y) (static_cast<void>(0))
#define TFLITE_DCHECK_LT(x, y) (static_cast<void>(0))
#define TFLITE_CHECK(condition) (static_cast<void>(0))
#define TFLITE_CHECK_EQ(x, y) (static_cast<void>(0))
#define TFLITE_CHECK_NE(x, y) (static_cast<void>(0))
#define TFLITE_CHECK_GE(x, y) (static_cast<void>(0))
#define TFLITE_CHECK_GT(x, y) (static_cast<void>(0))
#define TFLITE_CHECK_LE(x, y) (static_cast<void>(0))
#define TFLITE_CHECK_LT(x, y) (static_cast<void>(0))

#else  // not ifdef NDEBUG

#define TFLITE_ASSERT_FALSE TFLITE_ABORT
#define TF_LITE_FATAL(msg)  \
  do {                      \
    DEBUG_LOG(msg);         \
    DEBUG_LOG("\nFATAL\n"); \
    TFLITE_ABORT;           \
  } while (0)

#define TF_LITE_ASSERT(x)        \
  do {                           \
    if (!(x)) TF_LITE_FATAL(#x); \
  } while (0)

#define TF_LITE_ASSERT_EQ(x, y)                            \
  do {                                                     \
    if ((x) != (y)) TF_LITE_FATAL(#x " didn't equal " #y); \
  } while (0)

#define TFLITE_DCHECK(condition) (condition) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_EQ(x, y) ((x) == (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_NE(x, y) ((x) != (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_GE(x, y) ((x) >= (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_GT(x, y) ((x) > (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_LE(x, y) ((x) <= (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_DCHECK_LT(x, y) ((x) < (y)) ? (void)0 : TFLITE_ASSERT_FALSE
#define TFLITE_CHECK(condition) (condition) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_EQ(x, y) ((x) == (y)) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_NE(x, y) ((x) != (y)) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_GE(x, y) ((x) >= (y)) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_GT(x, y) ((x) > (y)) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_LE(x, y) ((x) <= (y)) ? (void)0 : TFLITE_ABORT
#define TFLITE_CHECK_LT(x, y) ((x) < (y)) ? (void)0 : TFLITE_ABORT

#endif

class RuntimeShape {
 public:
  // Shapes with dimensions up to 5 are stored directly in the structure, while
  // larger shapes are separately allocated.
  static constexpr int kMaxSmallSize = 5;

  RuntimeShape& operator=(RuntimeShape const&) = delete;

  RuntimeShape() : size_(0) {}

  explicit RuntimeShape(int dimensions_count) : size_(dimensions_count) {
    if (dimensions_count > kMaxSmallSize) {
      TFLITE_CHECK(false && "No shape resizing supported on this platform");
    }
  }

  RuntimeShape(int shape_size, int32_t value) : size_(0) {
    Resize(shape_size);
    for (int i = 0; i < shape_size; ++i) {
      SetDim(i, value);
    }
  }

  RuntimeShape(const std::initializer_list<int> init_list) : size_(0) {
    BuildFrom(init_list);
  }

  // Avoid using this constructor.  We should be able to delete it when C++17
  // rolls out.
  RuntimeShape(RuntimeShape const& other) : size_(other.DimensionsCount()) {
    std::memcpy(DimsData(), other.DimsData(), sizeof(int32_t) * size_);
  }

  ~RuntimeShape() {
    if (size_ > kMaxSmallSize) {
      TFLITE_CHECK(false && "No shape resizing supported on this platform");
    }
  }

  inline int32_t DimensionsCount() const { return size_; }
  inline void SetDimensionsCount(int32_t dimensions_count) { size_ = dimensions_count; }
  inline int32_t Dims(int i) const {
    TFLITE_DCHECK_GE(i, 0);
    TFLITE_DCHECK_LT(i, size_);
    return dims_[i];
  }
  inline void SetDim(int i, int32_t val) {
    TFLITE_DCHECK_GE(i, 0);
    TFLITE_DCHECK_LT(i, size_);
    dims_[i] = val;
  }

  inline int32_t* DimsData() { return dims_; }
  inline const int32_t* DimsData() const { return dims_; }

  inline void Resize(int dimensions_count) {
    if (dimensions_count > kMaxSmallSize) {
      TFLITE_CHECK(false && "No shape resizing supported on this platform");
    }
    size_ = dimensions_count;
  }

  template <typename T>
  inline void BuildFrom(const T& src_iterable) {
    const int dimensions_count =
        std::distance(src_iterable.begin(), src_iterable.end());
    Resize(dimensions_count);
    int32_t* data = DimsData();
    for (auto it : src_iterable) {
      *data = it;
      ++data;
    }
  }

  inline void BuildFrom(const std::initializer_list<int> init_list) {
    BuildFrom<const std::initializer_list<int>>(init_list);
  }

  // Returns the total count of elements, that is the size when flattened into a
  // vector.
  inline int FlatSize() const {
    int buffer_size = 1;
    const int* dims_data = reinterpret_cast<const int*>(DimsData());
    for (int i = 0; i < size_; i++) {
      buffer_size *= dims_data[i];
    }
    return buffer_size;
  }

 private:
  int32_t size_;
  int32_t dims_[kMaxSmallSize];
};

/* enum class FullyConnectedWeightsFormat : uint8_t { */
/*   kDefault, */
/*   kShuffled4x16Int8, */
/* }; */

struct FullyConnectedParams {
  // uint8_t inference params.
  // TODO(b/65838351): Use smaller types if appropriate.
  int32_t input_offset;
  int32_t weights_offset;
  int32_t output_offset;
  int32_t output_multiplier;
  int output_shift;
  // uint8_t, etc, activation params.
  int32_t quantized_activation_min;
  int32_t quantized_activation_max;
  // float activation params.
  /* float float_activation_min; */
  /* float float_activation_max; */
  /* // Mark the operands as cacheable if they are unchanging, e.g. weights. */
  /* bool lhs_cacheable; */
  /* bool rhs_cacheable; */
  /* FullyConnectedWeightsFormat weights_format; */
};

struct CMSISFullyConnectedParams {
  cmsis_nn_context ctx;
  cmsis_nn_fc_params fc_params;
  cmsis_nn_per_tensor_quant_params q_params;
  cmsis_nn_dims input_dims;
  cmsis_nn_dims filter_dims;
  cmsis_nn_dims bias_dims;
  cmsis_nn_dims output_dims;
};

struct ArithmeticParams {
  // Shape dependent / common to data / op types.
  /* BroadcastableOpCategory broadcast_category; */
  // uint8_t inference params.
  int32_t input1_offset;
  int32_t input2_offset;
  int32_t output_offset;
  int32_t output_multiplier;
  int output_shift;
  // Add / Sub, not Mul, uint8_t inference params.
  int left_shift;
  int32_t input1_multiplier;
  int input1_shift;
  int32_t input2_multiplier;
  int input2_shift;

  // TODO(b/158622529): Union the following activation params.
  // uint8_t, etc, activation params.
  int32_t quantized_activation_min;
  int32_t quantized_activation_max;
  /* // float activation params. */
  /* float float_activation_min; */
  /* float float_activation_max; */
  /* // int64_t activation params. */
  /* int64_t int64_activation_min; */
  /* int64_t int64_activation_max; */

  /* int broadcast_shape[5]; */
};

#endif  // __ABR_CUSTOM_TYPES_H__
