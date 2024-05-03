import numpy as np
import tensorflow as tf


def quant_range(details):
    scales = details["quantization_parameters"]["scales"]
    zeros = details["quantization_parameters"]["zero_points"]
    assert scales.size == zeros.size == 1
    return tuple((np.array([-128, 127]) - zeros) * scales)


def quantize(val, details):
    """Quantize the given value to int8."""

    val = np.asarray(val)
    scales = details["quantization_parameters"]["scales"]
    zeros = details["quantization_parameters"]["zero_points"]
    assert scales.size == zeros.size == 1
    assert scales.dtype == np.float32
    assert zeros.dtype == np.int32

    q = np.round(val / scales).astype(np.int32) + zeros
    return np.clip(q, -128, 127).astype(np.int8)


def dequantize(val, details):
    """Dequantize the given value to float32."""

    scales = details["quantization_parameters"]["scales"][0]
    zeros = details["quantization_parameters"]["zero_points"][0]
    assert scales.size == zeros.size == 1

    return (val.astype(np.float32) - zeros) * scales


class Model:
    def __init__(self, model_path):
        self.model_path = model_path

        self.interpreter = tf.lite.Interpreter(
            model_path=str(self.model_path.absolute())
        )
        self.interpreter.allocate_tensors()

        input_details = self.interpreter.get_input_details()
        output_details = self.interpreter.get_output_details()
        input_tensors = [
            self.interpreter.tensor(details["index"]) for details in input_details
        ]
        output_tensors = [
            self.interpreter.tensor(details["index"]) for details in output_details
        ]
        self.input_detail, self.state_input_detail = input_details
        self.output_detail, self.state_output_detail = output_details
        self.input_tensor, self.state_input_tensor = input_tensors
        self.output_tensor, self.state_output_tensor = output_tensors
        if len(self.input_detail["shape"]) == 2:
            inp_batch, inp_channels = self.input_detail["shape"]
            inp_steps = 1
        else:
            inp_batch, inp_steps, inp_channels = self.input_detail["shape"]
        assert inp_batch == 1 and inp_channels == 3
        # this model runs one step at a time, so that it can run online on the pod
        assert inp_steps == 1

        self.inp_batch = inp_batch
        self.inp_channels = inp_channels
        self.inp_steps = inp_steps

    def print_ranges(self):
        print(f"Input quant range: {quant_range(self.input_detail)}")
        print(f"State input quant range: {quant_range(self.state_input_detail)}")
        print(f"State output quant range: {quant_range(self.state_output_detail)}")
        print(f"Output quant range: {quant_range(self.output_detail)}")

    def quantize(self, signal):
        return quantize(signal, self.input_detail)

    def __call__(self, signal, quantized=None):
        assert signal.ndim == 2
        assert signal.shape[0] % self.inp_steps == 0
        assert signal.shape[1] == self.inp_channels

        # state starts at zero for each new signal
        state_init = np.zeros(self.state_input_detail["shape"], dtype=np.float32)

        quantized = signal.dtype.kind == "i" if quantized is None else quantized
        quant_signal = signal if quantized else self.quantize(signal)

        output = []
        for i in range(0, len(signal), self.inp_steps):
            # set inputs
            self.input_tensor()[:] = quant_signal[i : i + self.inp_steps]
            self.state_input_tensor()[:] = quantize(state_init, self.state_input_detail)

            # run sim
            self.interpreter.invoke()

            # get outputs
            out = dequantize(self.output_tensor(), self.output_detail)
            out = np.squeeze(out, axis=0)
            if out.ndim == 1:
                out = np.expand_dims(out, axis=0)

            output.append(out)

            # ensure that state persists (note that state input and output use different
            # quantization, so we have to dequantize and quantize again each iteration)
            state_init[:] = dequantize(
                self.state_output_tensor(), self.state_output_detail
            )

        output = np.concatenate(output, axis=0)
        if output.ndim == 1:
            output = np.expand_dims(output, axis=-1)

        return output
