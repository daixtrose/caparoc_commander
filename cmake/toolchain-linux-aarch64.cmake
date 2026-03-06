# CMake toolchain file for cross-compiling to Linux aarch64 (ARM64)
# Target: Raspberry Pi 5 / Revolution Pi Connect 5 (Cortex-A76, ARMv8.2-A)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc-14)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++-14)
set(CMAKE_STRIP aarch64-linux-gnu-strip)

# Use Bookworm aarch64 sysroot if available (e.g. inside cross-build container)
if(NOT DEFINED CMAKE_SYSROOT AND EXISTS "/opt/bookworm-arm64-sysroot")
    set(CMAKE_SYSROOT /opt/bookworm-arm64-sysroot)
endif()

# Search paths: find libraries/headers for target only, programs on host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
