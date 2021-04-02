# *****************************************************************************
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#  Author:
#      Hui Peng <peng124@purdue.edu>
#  Date:
#      2021-03-21
# *****************************************************************************/

cmake_minimum_required(VERSION 3.4.3)

project("SVF")

## add CPM
## https://github.com/cpm-cmake/CPM.cmake
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME Format.cmake
    VERSION 1.7.0
    GITHUB_REPOSITORY TheLartians/Format.cmake
    OPTIONS # set to yes skip cmake formatting
    "FORMAT_SKIP_CMAKE NO"
    # path to exclude (optional, supports regular expressions)
    "CMAKE_FORMAT_EXCLUDE cmake/CPM.cmake"
)

# LLVM_VER
if(DEFINED ENV{LLVM_VERSION})
    set(LLVM_VERSION $ENV{LLVM_VERSION})
else()
    set(LLVM_VERSION 10.0.0)
endif()

find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM_TOOLS_BINARY_DIR: ${LLVM_TOOLS_BINARY_DIR}")
message(STATUS "LLVM_ENABLE_RTTI: ${LLVM_ENABLE_RTTI}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add -std=gnu++17
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_FLAGS "-fPIC -Werror -Wall -Wextra -Wno-unused-parameter \
 -Wno-unused-variable")

# avoid reporting some warnings as error
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # SVF/lib/Util/TypeBasedHeapCloning.cpp:441:14:
    # error: variable 'clone' is used
    # uninitialized whenever 'if' condition is false
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-sometimes-uninitialized")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-maybe-uninitialized \
-Wno-sign-compare -Wno-unused-but-set-variable -Wno-cast-function-type \
-Wno-implicit-fallthrough")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} \
-Wno-overloaded-virtual -Wno-c++17-extensions")

if (NOT ${LLVM_ENABLE_RTTI})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O3")
endif()


list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


if (LLVM_LINK_LLVM_DYLIB)
    set(llvm_libs LLVM)
else()
    llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support transformutils)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}/include)

option(SVF_BUILD_UNITTESTS "Build all tests (default is OFF)" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include("svf_macros")

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})

# is Z3 really needed?
if(DEFINED ENV{Z3_DIR})
    set(Z3_DIR $ENV{Z3_DIR})
    include_directories(${Z3_DIR}/include)
    LINK_DIRECTORIES(${Z3_DIR}/bin)
else()
    message(WARNING "Z3_DIR is not set in env")
endif()

add_subdirectory(lib)
add_subdirectory(tools)

if (SVF_BUILD_UNITTESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/googletest")
        add_subdirectory(external/googletest)
        include_directories(external/googletest/googletest/include)
        add_subdirectory(tests)
        add_subdirectory(unittests)
    else()
        message(WARNING "external/googletest does not exist, "
            "please run git submodule init; git submodule update")
    endif()
endif()

INSTALL(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/include/ ${Z3_DIR}/include/
    COMPONENT devel
    DESTINATION include/svf
    FILES_MATCHING
    PATTERN "**/*.h"
    )

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Test-Suite")
    enable_testing()
    add_subdirectory(Test-Suite)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Test-Suite)
    include(CTest)
else()
    message("Test-Suite is not included in the build")
endif()
