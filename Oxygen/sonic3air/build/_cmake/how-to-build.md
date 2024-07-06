
# Build under Linux
Make sure the following packages are installed.
If needed, install them via "sudo apt-get install" (for Debian-based systems, this may differ for other Linux distributions).
- g++
- cmake
- libgl1-mesa-dev
- libglu1-mesa-dev
- libasound2-dev
- libpulse-dev
- libxcomposite-dev
- libxxf86vm-dev
- libcurl4-openssl-dev

=> Complete line to copy:
`sudo apt-get install g++ cmake libgl1-mesa-dev libglu1-mesa-dev libasound2-dev libpulse-dev libxcomposite-dev libxxf86vm-dev libcurl4-openssl-dev`

The following commands assume you start in the root directory of the S3AIR Git repo.

On first compilation, you need to create the build directory:
`mkdir ./Oxygen/sonic3air/build/_cmake/build`

## Build with CMake:
```
cd ./Oxygen/sonic3air/build/_cmake/build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

## Run Linux Build
Check if the CMake build created the `sonic3air_linux` binary in the correct location, namely inside `Oxygen/sonic3air`.
Place a copy of the S3&K ROM `Sonic_Knuckles_wSonic3.bin` inside `Oxygen/sonic3air` or `Oxygen/sonic3air/___internal`.
The following commands assume you're in the root directory of the S3AIR Git repo.