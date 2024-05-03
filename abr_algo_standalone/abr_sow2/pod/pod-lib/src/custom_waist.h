#ifndef __ABR_CUSTOM_WAIST_H__
#define __ABR_CUSTOM_WAIST_H__

/* ****************************************************************************
 * This function sets up the runtime and allocates all the required resources
 * for model execution.
 */
int custom_waist_setup(int input_size, int state_size, int output_size);

/* ****************************************************************************
 * This function sets pre-inference state(s) of the RNN(s) of the model and
 * should be called before each inference.
 *
 * The states are treated as inputs and outputs to the model and may have
 * different quantization parameters on the input- and output-sides which
 * therefore requires the states to be dequantized and requantized for each
 * inference.
 *
 * state_vals: Pointer to memory buffer from which pre-inference states are
 *             passed into the model. This function consumes ``kStateInputSize``
 *             elements from the buffer.
 */
void custom_waist_set_states(float* state_vals);

/* ****************************************************************************
 * This function gets post-inference state(s) of the RNN(s) of the model and
 * should be called after each inference to loop states back to the input-side.
 *
 * The states are treated as inputs and outputs to the model and may have
 * different quantization parameters on the input- and output-sides which
 * therefore requires the states to be dequantized and requantized for each
 * inference.
 *
 * state_vals: Pointer to memory buffer to which post-inference states from the
 *             model are written. This function writes ``kStateInputSize``
 *             elements to the buffer.
 */
void custom_waist_get_states(float* state_vals);

/* ****************************************************************************
 * This function sets the model inputs and should be called before each
 * inference.
 *
 * Internally this performs quantization of the input values before passing them
 * to the model.
 *
 * input_vals: Pointer to memory buffer from which input values passed into the
 *             model. This function consumes ``kModelInputSize`` elements from
 *             the buffer.
 */
void custom_waist_set_inputs(float* input_vals);

/* ****************************************************************************
 * This function gets the model outputs and should be called after each
 * inference.
 *
 * Internally this performs dequantization of the output values before
 * retrieving them from the model.
 *
 * output_vals: Pointer to memory buffer to which output values from the model
 *              are written. This function writes ``kOutputSize`` elements to
 *              the buffer.
 */
void custom_waist_get_outputs(float* output_vals);

/* ****************************************************************************
 * This function performs inference and updates the states and outputs.
 */
int custom_waist_inference();

#endif  // __ABR_CUSTOM_WAIST_H__
