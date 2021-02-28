//=============================================================================
//  A class to aid testing of polynomials for maximality
//----------------------------------------------------------------------------
//  This file is part of MLPolyGen, a maximal-length polynomial generator
//  for linear feedback shift registers.
//  
//  Copyright (C) 2012  Gregory E. Allen
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#include "MLPolyTester.h"
#include <stdio.h>
#include <stdarg.h>


//-----------------------------------------------------------------------------
int MLPolyTesterBase::dbprintf(unsigned level, const char *fmt, ...) const
//-----------------------------------------------------------------------------
{
    va_list val;
    int result = 0;
    if (level<=verbosity) {
        va_start(val, fmt);
        result = vfprintf(stderr, fmt, val);
        va_end(val);
    }
    return result;
}
