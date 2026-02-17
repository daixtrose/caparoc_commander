# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-src"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-build"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/tmp"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/src/libmodbus_cpp_proj-populate-stamp"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/src"
  "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/src/libmodbus_cpp_proj-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/src/libmodbus_cpp_proj-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/markus/PROJECTS/caparoc-utils/caparoc_commander/build-fetchcheck/_deps/libmodbus_cpp_proj-subbuild/libmodbus_cpp_proj-populate-prefix/src/libmodbus_cpp_proj-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
