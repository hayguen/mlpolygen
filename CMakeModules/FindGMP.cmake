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

if(GMP_INCLUDE_DIR)
  set(GMP_FIND_QUIETLY TRUE)
endif()

find_path(
    GMP_INCLUDE_DIR
    gmp.h
    PATHS "${GMP_ROOT}/include"
    NO_DEFAULT_PATH)

find_path(GMP_INCLUDE_DIR gmp.h)

find_library(
    GMP_LIBRARY
    NAMES gmp
    PATHS "${GMP_ROOT}/lib"
    NO_DEFAULT_PATH)

find_library(GMP_LIBRARY NAMES gmp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_LIBRARY GMP_INCLUDE_DIR)

if(GMP_FOUND)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
else(GMP_FOUND)
  set(GMP_LIBRARIES)
endif(GMP_FOUND)

mark_as_advanced(GMP_LIBRARY GMP_INCLUDE_DIR)

