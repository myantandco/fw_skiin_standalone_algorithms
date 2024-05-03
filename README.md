# Skiin Firmware Standalone Algorithms
Repository to contain all algorithms in Skiin FW as standalone applications.

# Purpose
These standalone applications are provided to data science/systems to be able verify the functionality of the C implementation before we port changes in the FW application repository. This also enabled us to develop unit tests for algorithms as required.

Please note, currently these algorithms use `double` type variables but this should be abandoned as the hardware only supports single-precision floating point values. This means all `doubles` will be truncated to 7 decimal places when running on the target device (i.e. Pod) which may cause noise/errors/artifacts in the output or even differences in behavior when running on PC vs the device. Thus, all `double` type variables should be converted to `float` type.

Please speak with firmware team before implementing any changes to the input and output of these algorithms as it impacts how data will be received from sensors or transmitted over BLE. The internal functions can be modified if and when required. 

# Installation
Certain algorithms depend on GCC and others use MinGW depending on the whether the build needs cross-compilation.

## To install MinGW:

1. Install chocolatey:
2. Install MinGW:

## To install GCC:

1. Install gcc:

# Usage

After your environment is configured, simply run `make` from the command line from the selected algorithm folder. The build will automatically be generated and placed in the `/build` directory. 

# Future Improvements

- Find a way to limit the use of doubles and provide warnings when they are used
- Convert all doubles to floats
- Develop methods to convert CSV files containing input data into arrays for applications to consume
- Develop methods to output data to file rather than via logging
- Should python implementations of these algorithms be included in this repo or somewhere else?
