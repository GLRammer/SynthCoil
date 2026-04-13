#!/usr/bin/env bash

# rebuild config
if [ "$1" == "config" ]; then
    cmake -S . -B wbuild -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/home/zero/SynthCoil/windows-toolchain.cmake
fi

# delete current build and rebuild config
if [ "$1" == "scratch" ]; then
    rm -rf wbuild
    cmake -S . -B wbuild -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/home/zero/SynthCoil/windows-toolchain.cmake
fi

# build project with as many parallel processes as you have processors
cmake --build wbuild/ -j "$(nproc)"

# open Powershell in build directory
if [ "$1" == "p" ]; then
    powershell.exe -Command "Start-Process powershell -ArgumentList '-NoExit', '-Command', \"cd .\\wbuild\""
fi

# execute built program in Powershell
# This automatically closes Powershell when the program closes
if [ "$1" == "e" ]; then
    powershell.exe -Command "Start-Process powershell -ArgumentList '-Command', \"cd .\\wbuild; .\\synthDemo.exe\""
fi