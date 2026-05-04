# SynthCoil
Long term intent is to build a program that generates a dynamic 3D object utilizing an audio input.
## Dependencies
Vendored
- FFTW - float version
- SDL3 
- DearImGUI
- GLM

Not vendored
- Cmake
- vcpkg
- Vulkan
- glslc (Should be packaged with Vulkan)
### Portaudio
Used for capturing inputs, has multiple audio libraries as its own dependencies.
### FFTW
Used for converting audio stream into frequencies for audio analysis.
### DearImGUI + SDL3 + Vulkan
Graphical engine of choice.
### Vulkan install with vcpkg
if cross compiling for windows, run `vcpkg install vulkan:x64-mingw-dynamic`. This assumes vcpkg is already in your path.
### glslc
Used to compile shaders into .spv files, `CMakeLists.txt` assumes it is installed with vulkan, if this is incorrect, comment out line 46 and uncomment line 47.
### GLM
Used for graphics matrices. Uses system GLM by default, but is also vendored.
## Compiling
All compilation setups assume you are running the commands from the 
### Windows or Linux 
Requires Cmake, Vulkan, and glslc to be installed on your system and available to your path.
In the repo directory run `cmake -S . -B build` and `cmake --build build`, but add `-G ` and the compiler of your choice in quotes when building on Windows.
Feel free to add `-j <int>` to the second command where `<int>` is the number of parallel threads to use to build the program.

VSCode's CMake extension can also build the project easily!
### Cross-compile Linux to Windows
A Linux system can build *for* Windows by adding `-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=./windows-toolchain.cmake` to your CMake configure command (assuming you have vcpkg and installed vulkan using it).
## Execution
Currently prompts user for an audio device from a drop-down menu, then continuously grabs and processes audio while rendering a shape to the screen and a control panel. Current rendering is similar to final expectations, but more customization is desired.

In a terminal of choice run `./build/synthDemo` for Linux or `.\build\synthDemo.exe` on Windows.