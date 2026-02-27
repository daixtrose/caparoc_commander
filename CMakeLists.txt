cmake_minimum_required(VERSION 3.25...4.2)

project(caparoc_commander
    VERSION 0.1.0
    DESCRIPTION "CLI commander for CAPAROC systems"
    LANGUAGES CXX
)

include(FetchContent)
include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Dependencies – rebuilt from source
# ---------------------------------------------------------------------------
if(NOT TARGET caparoc)
    FetchContent_Declare(
        libcaparoc_proj
        GIT_REPOSITORY https://github.com/daixtrose/libcaparoc.git
        GIT_TAG        v0.1.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(libcaparoc_proj)
endif()

if(NOT TARGET CLI11::CLI11)
    FetchContent_Declare(
        cli11_proj
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
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

# ---------------------------------------------------------------------------
# Install & CPack
# ---------------------------------------------------------------------------
install(TARGETS caparoc_commander
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(CPACK_PACKAGE_NAME            "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION         "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION     "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_CONTACT         "daixtrose")
set(CPACK_GENERATOR               "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS  "libmodbus5")
include(CPack)
