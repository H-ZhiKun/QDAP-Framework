#!/bin/bash

rm -rf build

mkdir build

cd build

cmake  -G Ninja -S .  -DCMAKE_TOOLCHAIN_FILE=/home/user/code/vcpkg_inner/vcpkg/scripts/buildsystems/vcpkg.cmake ..

ninja -j6


cd -
