# CMake toolchain file for cross-compiling to Linux aarch64 (ARM64).
# Target: Raspberry Pi 5 / Revolution Pi Connect 5 (Cortex-A76, ARMv8.2-A)
#         running Debian Trixie (glibc 2.41).
#
# Expected build environment: the `ghcr.io/daixtrose/cross-build-trixie-arm64`
# container.  The previous Bookworm sysroot path
# (/opt/bookworm-arm64-sysroot) has been dropped.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc-14)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++-14)
set(CMAKE_STRIP        aarch64-linux-gnu-strip)

if(NOT DEFINED CMAKE_SYSROOT AND EXISTS "/opt/trixie-arm64-sysroot")
    set(CMAKE_SYSROOT /opt/trixie-arm64-sysroot)
    message(STATUS "Using Trixie aarch64 sysroot: ${CMAKE_SYSROOT}")
endif()

set(CMAKE_CROSSCOMPILING_EMULATOR /usr/bin/qemu-aarch64-static)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
