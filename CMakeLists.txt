# This file is part of MLPolyGen, a maximal-length polynomial generator
# for linear feedback shift registers.
# 
# Copyright (C) 2012  Gregory E. Allen
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.15)

# required to enable set_property(TARGET clsocket PROPERTY  MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)
# set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(mlpolygen)

option(WITHOUT_GMP "do not use GMP, the GNU Multiple Precision Arithmetic Library?" OFF)


# set the reported version for mlpolygen
add_definitions("-DMLPOLYGEN_VERSION=\"1.1.0\"")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
  # using Visual Studio C++
  message(STATUS "INFO: detected MSVC: will not link math lib m")
  set(MATHLIB "")

  add_definitions("/D_CRT_SECURE_NO_WARNINGS")
  set(MSVC_DISABLED_WARNINGS_LIST "C4996")

else()
  message(STATUS "INFO: detected NO MSVC: ${CMAKE_C_COMPILER_ID}: will link math lib m")
  set(MATHLIB "m")
endif()


if (NOT WITHOUT_GMP)
    find_package(GMP)
    find_package(GMPXX)
    if (NOT GMPXX_FOUND)
        message("## GMP dependency can be disabled with cmake option -DWITHOUT_GMP=1")
        message(FATAL_ERROR "Could not find GMP, the GNU Multiple Precision Arithmetic Library.")
    endif (NOT GMPXX_FOUND)
else (NOT WITHOUT_GMP)
    message(STATUS "GMP dependency has been disabled")
endif (NOT WITHOUT_GMP)

include_directories(libs/PrimeFactorizer)
add_executable(mlpolygen src/main.cc src/MLPolyTester.cc)
add_executable(PrimeFactorizer libs/PrimeFactorizer/main.cc)
add_executable(lfsr_s src/lfsr_s.c)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cargs/CMakeLists.txt")
  message(FATAL_ERROR
    "you forgot the '--recursive' flag at git clone\n"
    "  you can proceed with following commands:\n"
    "  $  git submodule init\n"
    "  $  git submodule update"
  )
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/cargs")

if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
    # make a static compilation with MSVC
    set_property(TARGET mlpolygen       PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET PrimeFactorizer PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET lfsr_s          PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    set_property(TARGET cargs           PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET cargstest       PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    set_target_properties(cargs     PROPERTIES COMPILE_FLAGS "/wd5105")
    set_target_properties(cargstest PROPERTIES COMPILE_FLAGS "/wd5105")
endif()

target_link_libraries(mlpolygen cargs)


if (GMPXX_FOUND)
    include_directories(${GMPXX_INCLUDE_DIR})
    target_link_libraries(mlpolygen ${GMP_LIBRARIES} ${GMPXX_LIBRARIES})
    target_link_libraries(PrimeFactorizer ${GMP_LIBRARIES} ${GMPXX_LIBRARIES})
    add_definitions( -DUSING_GMP=1 )
endif (GMPXX_FOUND)

target_link_libraries(lfsr_s ${MATHLIB})

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)

install(TARGETS mlpolygen RUNTIME DESTINATION bin)
