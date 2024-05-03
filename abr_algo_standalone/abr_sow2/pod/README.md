# Pod Model Deployment

This example is based on the [Nordic SDK UART example](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/uart_example.html)
and the [TFLM hello_world example](https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/hello_world).
This example uses an optimized R-peak detector model and illustrates how to
to use the interface to provide inputs and read outputs. Output values are sent
to the UART to confirm execution of the model.


## Key Components

- ``libabr-myant-arm-rpeak.a``: Library containing the runtime and an
  optimized version of the R-peak detector model.
- ``tflm.h``: Header file describing the interface to the model and runtime.
- ``constants.h``: Model size parameters used to prepare expected data shapes.
- ``data.h``: This holds the sample inputs and expected outputs.
  - The input data is from TIT01B-ID01-T1, specifically from seconds 5 to 15.
  - The reference outputs are from running this input data on the nRF52840
    device using our standard development build.


## Usage

This example was created and tested with **Ubuntu 20.04** using
**arm-none-eabi-gcc v10.2.1** and **Nordic SDK v17.0.2** on the
**nRF52840** device.

### Requirements

This example requires ``make``, ``arm-none-eabi-gcc >= 10``, and the
[Nordic SDK](https://www.nordicsemi.com/Products/Development-software/nrf5-sdk/download#infotabs).

An easy way to download ``arm-none-eabi-gcc >= 10`` is to make use of the
[tflite-micro download script](https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/tools/make/arm_gcc_download.sh):

1. Clone the repo ``git clone https://github.com/tensorflow/tflite-micro.git``
2. Navigate to ``tflite-micro/tensorflow/lite/micro/tools/make``
3. Run the download script providing a target download directory,
   e.g. ``./arm_gcc_download.sh /home/$USER/Downloads``

### Deployment

1. Confirm the path for ``arm-none-eabi-gcc``. By default ``make`` will search
   in ``TARGET_TOOLCHAIN_ROOT=/usr/bin``. If necessary, update this path in the
   Makefile on line 8 or override using a ``TARGET_TOOLCHAIN_ROOT`` environment
   variable. If the compiler was downloaded to ``/home/$USER/Downloads`` as per
   above, try ``TARGET_TOOLCHAIN_ROOT=/home/$USER/Downloads/gcc_embedded/bin``.
   Note that there is **no trailing slash** in the path.
2. Provide a path to the Nordic SDK. Set the ``SDK_ROOT`` path in the Makefile
   on line 3 or supply a ``SDK_ROOT`` environment variable. Note that there is
   **no trailing slash** in the path.
3. Run ``make`` to build the executable.
4. Connect the nRF52840 to the host computer.
5. Open a serial terminal to watch the device output configured with 115200 8N1
   (e.g. ``sudo minicom -D /dev/ttyACM0``).
6. Program the device (e.g. ``cp abr-myant-arm-rpeak-demo.hex /media/ben/JLINK``).
7. Observe the output. The program will output reference and prediction values
   for each input. The final output should read as follows:
```
Overall 320/320 outputs were within 0.0001
```
