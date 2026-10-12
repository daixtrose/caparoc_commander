cmake_minimum_required(VERSION 3.25...4.2)

project(caparoc_commander
    VERSION 1.0.7
    DESCRIPTION "CLI commander for CAPAROC systems"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(FetchContent)

option(CAPAROC_COMMANDER_ENABLE_CPACK "Enable CPack packaging support" ${PROJECT_IS_TOP_LEVEL})
option(CAPAROC_COMMANDER_BUILD_TESTS "Build the caparoc_commander tests" ${PROJECT_IS_TOP_LEVEL})

# ---------------------------------------------------------------------------
# Dependencies – rebuilt from source
# ---------------------------------------------------------------------------
if(NOT TARGET caparoc)
    set(LIBMODBUS_CPP_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        libcaparoc_proj
        GIT_REPOSITORY https://github.com/daixtrose/libcaparoc.git
        GIT_TAG        v0.6.2
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(libcaparoc_proj)
endif()

if(NOT TARGET CLI11::CLI11)
    FetchContent_Declare(
        cli11_proj
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.6.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(cli11_proj)
endif()

# cpp-httplib (header only): the client for the module's web interface. Plain HTTP only — the
# modules serve no TLS — so neither OpenSSL nor compression is linked, and the package gains no
# run-time dependency.
if(NOT TARGET httplib::httplib)
    FetchContent_Declare(
        httplib_proj
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG        v0.18.3
        GIT_SHALLOW    TRUE
        SYSTEM
    )
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE    OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_BROTLI_IF_AVAILABLE  OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_INSTALL                  OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(httplib_proj)
endif()

# nlohmann/json (header only): the module's network settings are a JSON object.
if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(
        nlohmann_json_proj
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.12.0
        GIT_SHALLOW    TRUE
        SYSTEM
    )
    set(JSON_Install OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json_proj)
endif()

# ---------------------------------------------------------------------------
# Web interface of the module (network settings), a library of its own so
# the tests link it without the Modbus side
# ---------------------------------------------------------------------------
add_library(caparoc_commander_webif STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/network_config.cpp
)

target_include_directories(caparoc_commander_webif
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(caparoc_commander_webif
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
)

target_compile_features(caparoc_commander_webif PUBLIC cxx_std_23)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(caparoc_commander
    ${CMAKE_CURRENT_LIST_DIR}/src/caparoc_commander.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/cli_parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/create_modbus_connection.cpp
)

set_target_properties(caparoc_commander PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_include_directories(caparoc_commander
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(caparoc_commander
    PRIVATE
        caparoc
        caparoc_commander_webif
        CLI11::CLI11
)

target_compile_features(caparoc_commander PRIVATE cxx_std_23)

foreach(target IN ITEMS caparoc_commander caparoc_commander_webif)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    elseif(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
        )
    endif()
endforeach()

# Statically link MinGW runtime (libstdc++, libgcc, libwinpthread) so the
# resulting .exe is fully self-contained and runs without MSYS2/MinGW installed.
if(MINGW)
    target_link_options(caparoc_commander PRIVATE -static)
endif()

# Trixie ships GCC 14 libstdc++, so no static-libstdc++ kludge needed
# any more — the deb declares libstdc++6 as a runtime Depends:.

if(CAPAROC_COMMANDER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CMakeListsCPackConfiguration.txt)
