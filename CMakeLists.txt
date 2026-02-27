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
# Self-contained install configuration
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "caparoc-commander")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Install to private prefix under /opt
    set(INSTALL_PREFIX "/opt/daixtrose/${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}")
    set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE PATH "" FORCE)
    set(CPACK_PACKAGING_INSTALL_PREFIX ${INSTALL_PREFIX})

    # RUNPATH: executable finds shared libs relative to its own location
    file(RELATIVE_PATH _libdir_rel
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    )
    set_target_properties(caparoc_commander PROPERTIES
        BUILD_RPATH_USE_ORIGIN ON
        INSTALL_RPATH "$ORIGIN:$ORIGIN/${_libdir_rel}"
    )
endif()

install(TARGETS caparoc_commander
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Ship the libmodbus shared library built from source via ExternalProject
install(DIRECTORY "${CMAKE_BINARY_DIR}/_deps/libmodbus-install/lib/"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILES_MATCHING
    PATTERN "libmodbus.so*"
    PATTERN "pkgconfig" EXCLUDE
)

set(CPACK_PACKAGE_VENDOR "Markus Werle")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_CONTACT "daixtrose")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Markus Werle")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_VERBATIM_VARIABLES YES)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR TGZ DEB)

    # Debian maintainer scripts: symlink into /usr/local/bin
    set(EXECUTABLE_NAME "caparoc_commander")
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/packaging/postinst.in
        ${CMAKE_CURRENT_BINARY_DIR}/packaging/postinst
        @ONLY
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/packaging/prerm.in
        ${CMAKE_CURRENT_BINARY_DIR}/packaging/prerm
        @ONLY
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
    list(APPEND CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_CURRENT_BINARY_DIR}/packaging/postinst"
        "${CMAKE_CURRENT_BINARY_DIR}/packaging/prerm"
    )
else()
    set(CPACK_GENERATOR TGZ)
endif()

include(CPack)
