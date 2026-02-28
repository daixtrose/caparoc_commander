cmake_minimum_required(VERSION 3.25...4.2)

project(caparoc_commander
    VERSION 0.3.0
    DESCRIPTION "CLI commander for CAPAROC systems"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(FetchContent)

option(CAPAROC_COMMANDER_ENABLE_CPACK "Enable CPack packaging support" ${PROJECT_IS_TOP_LEVEL})

# ---------------------------------------------------------------------------
# Dependencies – rebuilt from source
# ---------------------------------------------------------------------------
if(NOT TARGET caparoc)
    set(LIBMODBUS_CPP_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        libcaparoc_proj
        GIT_REPOSITORY https://github.com/daixtrose/libcaparoc.git
        GIT_TAG        v0.3.0
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
        CLI11::CLI11
)

target_compile_features(caparoc_commander PRIVATE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(caparoc_commander PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
elseif(MSVC)
    target_compile_options(caparoc_commander PRIVATE
        /W4
    )
endif()

include(${CMAKE_CURRENT_LIST_DIR}/CMakeListsCPackConfiguration.txt)
