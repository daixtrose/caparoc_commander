# caparoc_commander

A command-line tool for communicating with
[Phoenix Contact CAPAROC](https://www.phoenixcontact.com/en-us/products/caparoc)
circuit-breaker systems over Modbus TCP.
It exposes every register documented in the CAPAROC Modbus register map and
adds high-level convenience commands for day-to-day device management.

## Table of Contents

- [Features](#features)
- [Command-Line Reference](#command-line-reference)
  - [Connection Options](#connection-options)
  - [Register Discovery](#register-discovery)
  - [Generic Register Access](#generic-register-access)
  - [Coil Access](#coil-access)
  - [Device Information](#device-information)
  - [System and Channel Status](#system-and-channel-status)
  - [Channel Control](#channel-control)
  - [Nominal Current Management](#nominal-current-management)
  - [Reset Commands](#reset-commands)
  - [Miscellaneous](#miscellaneous)
- [Prerequisites](#prerequisites)
- [Building with CMake Presets](#building-with-cmake-presets)
  - [Available Presets](#available-presets)
  - [Linux Native Build](#linux-native-build)
  - [Linux aarch64 Cross-Compilation](#linux-aarch64-cross-compilation)
  - [Windows Build (MSYS2 MinGW)](#windows-build-msys2-mingw)
  - [Packaging](#packaging)
- [Installation from Packages](#installation-from-packages)
- [License](#license)

## Features

- **Full register map** – list, search and inspect all 771 CAPAROC Modbus
  registers from the command line.
- **Generic read/write** – access any UINT16, UINT32 or STRING32 register by
  hex address.
- **Coil access** – read and write Modbus coils.
- **High-level commands** – product names, system status, channel status,
  load current, channel on/off control, nominal current get/set/unlock, and
  bulk reset operations.
- **Cross-platform** – builds on Linux (x86_64 & aarch64) and Windows (MSYS2
  MinGW-w64).

## Command-Line Reference

```
caparoc_commander [OPTIONS]
```

Multiple action flags can be combined in a single invocation; they are
executed in the order given.

### Connection Options

| Flag | Description | Default |
|------|-------------|---------|
| `-i, --ip ADDRESS` | IP address of the CAPAROC device | `192.168.1.2` |
| `-p, --port PORT` | Modbus TCP port | `502` |
| `-t, --timeout SECONDS` | Connection timeout in seconds | `3` |
| `-d, --debug` | Enable debug output | off |
| `-h, --help` | Show all available options | |

### Register Discovery

| Flag | Description |
|------|-------------|
| `-l, --list` | List every register in the CAPAROC register map |
| `-r, --register ADDRESS` | Show detailed info for a single register (hex, e.g. `0x0010`) |
| `-s, --search PATTERN` | Search registers by name (case-insensitive substring match) |

**Examples:**

```bash
# List all 771 registers
caparoc_commander -l

# Show info for register 0x0010
caparoc_commander -r 0x0010

# Find all voltage-related registers
caparoc_commander -s voltage
```

### Generic Register Access

| Flag | Arguments | Description |
|------|-----------|-------------|
| `--read-uint16 ADDRESS` | hex address | Read a single UINT16 register |
| `--read-uint32 ADDRESS` | hex address | Read a single UINT32 register |
| `--read-string32 ADDRESS` | hex address | Read a STRING32 (16-word) register |
| `--write-uint16 ADDRESS VALUE` | hex address, integer value | Write a UINT16 register |
| `--write-uint32 ADDRESS VALUE` | hex address, integer value | Write a UINT32 register |

**Examples:**

```bash
# Read input voltage register
caparoc_commander --read-uint16 0x2004

# Write value 1 to register 0xC000
caparoc_commander --write-uint16 0xC000 1
```

### Coil Access

| Flag | Arguments | Description |
|------|-----------|-------------|
| `--read-coil ADDRESS` | integer or hex address | Read coil status (ON/OFF) |
| `--write-coil ADDRESS STATE` | address + `on\|off\|true\|false\|1\|0` | Write coil state |

**Examples:**

```bash
caparoc_commander --read-coil 0x0001
caparoc_commander --write-coil 0x0001 on
```

### Device Information

| Flag | Description |
|------|-------------|
| `--print-device-info` | Print device information (modules, product names, channels) |
| `--num-connected-modules` | Get the number of currently connected modules |
| `--product-name-power-module` | Get the product name of the Power Module |
| `--product-name-module N [N ...]` | Get the product name of module(s) 1–16 |
| `--product-name-quint` | Get the product name of the QUINT Power Supply |

**Examples:**

```bash
# Full device overview
caparoc_commander --print-device-info

# Product name of modules 1 and 2
caparoc_commander --product-name-module 1 2
```

### System and Channel Status

| Flag | Arguments | Description |
|------|-----------|-------------|
| `--get-system-status` | — | Show system-level voltage, current, temperature and flags |
| `--get-channel-status M C` | module, channel | Show status flags for a specific channel |
| `--get-load-current M C` | module, channel | Get the actual load current of a channel |

**Examples:**

```bash
# System overview
caparoc_commander --get-system-status

# Status and current of module 1, channel 2
caparoc_commander --get-channel-status 1 2 --get-load-current 1 2
```

### Channel Control

| Flag | Arguments | Description |
|------|-----------|-------------|
| `--control-channel M C STATE` | module, channel, `on\|off` | Switch a channel on or off |

**Example:**

```bash
# Turn module 1 channel 3 off, then back on
caparoc_commander --control-channel 1 3 off
caparoc_commander --control-channel 1 3 on
```

### Nominal Current Management

| Flag | Arguments | Description |
|------|-----------|-------------|
| `--get-nominal-current M C` | module, channel | Read the configured nominal current |
| `--set-nominal-current M C VALUE` | module, channel, amperes | Set the nominal current |
| `--unlock-nominal-current M C` | module, channel | Unlock nominal current parametrization (global + channel lock) |

**Example:**

```bash
# Unlock, then set nominal current for module 1 channel 1 to 4 A
caparoc_commander --unlock-nominal-current 1 1 --set-nominal-current 1 1 4
```

### Reset Commands

| Flag | Description |
|------|-------------|
| `--reset-application-params-power-and-cb` | Reset application parameters for Power Module and Circuit Breakers |
| `--global-channel-error-reset-all-cb` | Global channel error reset for all Circuit Breakers |
| `--error-counter-reset-all-cb` | Reset error counters for all Circuit Breakers |
| `--reset-application-params-quint` | Reset application parameters for the QUINT Power Supply |

### Miscellaneous

| Flag | Description |
|------|-------------|
| `-d, --debug` | Print connection details and the parsed command-line options |
| `-h, --help` | Show the built-in CLI11 help text |

## Prerequisites

| Requirement | Notes |
|---|---|
| CMake ≥ 3.25 | Tested with CMake 4.2.3 |
| C++23 compiler | GCC 14+ or equivalent |
| `pkg-config` | For finding libmodbus |
| autotools (`autoconf`, `automake`, `libtool`) | Required to build the vendored libmodbus |

The following dependencies are **fetched automatically** via CMake
`FetchContent` – no manual installation necessary:

- [libcaparoc](https://github.com/daixtrose/libcaparoc) (v0.4.0) – CAPAROC register map and high-level API
- [libmodbus-cpp](https://github.com/daixtrose/libmodbus-cpp) (v1.4.0) – C++ Modbus wrapper (itself vendoring [libmodbus](https://github.com/stephane/libmodbus))
- [CLI11](https://github.com/CLIUtils/CLI11) (v2.6.1) – command-line parser

### Debian / Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y g++-14 pkg-config autoconf automake libtool make
```

### MSYS2 MinGW (Windows)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          autoconf automake libtool make pkg-config gettext git
```

## Building with CMake Presets

The project ships a `CMakePresets.json` (schema version 8) with ready-made
configure, build and package presets.

### Available Presets

| Configure Preset | Build Preset | Package Preset | Platform |
|---|---|---|---|
| `linux-x86_64-release` | `linux-x86_64-release` | `linux-x86_64-release` | Linux x86_64 |
| `linux-x86_64-debug` | `linux-x86_64-debug` | — | Linux x86_64 (debug) |
| `linux-aarch64-release` | `linux-aarch64-release` | `linux-aarch64-release` | Linux aarch64 (cross) |
| `windows-x86_64-release` | `windows-x86_64-release` | `windows-x86_64-release` | Windows x86_64 (MSYS2) |

### Linux Native Build

```bash
# Configure
cmake --preset linux-x86_64-release

# Build
cmake --build --preset linux-x86_64-release --parallel

# Run
./build/linux-x86_64-release/bin/caparoc_commander --help
```

For a debug build replace `linux-x86_64-release` with `linux-x86_64-debug`.

### Linux aarch64 Cross-Compilation

Requires the `aarch64-linux-gnu` cross toolchain (e.g.
`g++-14-aarch64-linux-gnu`):

```bash
cmake --preset linux-aarch64-release
cmake --build --preset linux-aarch64-release --parallel
```

### Windows Build (MSYS2 MinGW)

Open an **MSYS2 MinGW64** shell:

```bash
cmake --preset windows-x86_64-release
cmake --build --preset windows-x86_64-release --parallel
./build/windows-x86_64-release/bin/caparoc_commander.exe --help
```

### Packaging

Generate platform packages with the corresponding package preset:

```bash
# Linux: creates .tar.gz, .deb and .rpm
cpack --preset linux-x86_64-release

# Linux aarch64: same generators, cross-compiled
cpack --preset linux-aarch64-release

# Windows: creates a .zip archive
cpack --preset windows-x86_64-release
```

Packages are self-contained and install under
`/opt/daixtrose/caparoc-commander-<version>/`.
On Debian-based systems the `.deb` package automatically symlinks the
executable into `/usr/local/bin/`.

## Installation from Packages

### Debian / Ubuntu (.deb)

```bash
sudo dpkg -i caparoc-commander_0.4.0_amd64.deb
caparoc_commander --help
```

### RPM-based (Fedora, RHEL, openSUSE)

```bash
sudo rpm -i caparoc-commander-0.4.0-1.x86_64.rpm
```

### Windows

Unzip the archive and run `caparoc_commander.exe` from the `bin/` directory.

## License

This project is licensed under the [MIT License](LICENSE).
