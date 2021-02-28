# vim:ts=4:sw=4:expandtab:autoindent:
#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

if(GMPXX_INCLUDE_DIR)
  set(GMPXX_FIND_QUIETLY TRUE)
endif()

if(NOT GMPXX_ROOT)
    set(GMPXX_ROOT ${GMP_ROOT})
endif()

find_path(
    GMPXX_INCLUDE_DIR
    gmp.h
    PATHS "${GMPXX_ROOT}/include"
    NO_DEFAULT_PATH)

find_path(GMPXX_INCLUDE_DIR gmpxx.h)

find_library(
    GMPXX_LIBRARY
    NAMES gmpxx
    PATHS "${GMP_ROOT}/lib"
    NO_DEFAULT_PATH)

find_library(GMPXX_LIBRARY NAMES gmpxx)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMPXX DEFAULT_MSG GMPXX_LIBRARY GMPXX_INCLUDE_DIR)

if(GMPXX_FOUND)
  set(GMPXX_LIBRARIES ${GMPXX_LIBRARY})
else(GMPXX_FOUND)
  set(GMPXX_LIBRARIES)
endif(GMPXX_FOUND)

mark_as_advanced(GMPXX_LIBRARY GMPXX_INCLUDE_DIR)

