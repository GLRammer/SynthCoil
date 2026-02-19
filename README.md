# SynthCoil
Long term intent is to build a program that generates a dynamic 3D object utilizing an audio input.
## Dependencies
### Portaudio
Used for capturing inputs, has multiple audio libraries as its own dependencies.
### FFTW
Used for converting audio stream into frequencies for audio analysis.
### Vulkan
Graphical engine of choice.
## Compiling
### Linux
Should just require CMake, as the dependencies are in the `vendor/` directory
In the repo directory run `cmake -S . -B build` and `cmake --build build`.
Feel free to add `-j <int>` to the second command where `<int>` is the number of parallel threads to use to build the program.
### Windows
Currently unable to build *on* Windows, but a Linux system can build *for* Windows by adding `-DCMAKE_TOOLCHAIN_FILE=./windows-toolchain.cmake` to the first CMake command.
## Execution
Currently grabs 5 seconds of live audio after verifying audio input. Does not currently take any arguments and user input is currently limited to yes or no inputs in the command line.
In a terminal of choice run `./build/synthDemo` for Linux or `.\build\synthDemo.exe` on Windows.