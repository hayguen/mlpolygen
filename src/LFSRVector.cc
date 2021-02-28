//=============================================================================
//  A class for a vector of polynomials, a square matrix of bits
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

#include "LFSRVector.h"

int test_LFSRVector()
{
    typedef std::bitset<128> test_poly_t;

    LFSRVector<test_poly_t> vec0(12);
    //std::cout << vec << std::endl;
    
    LFSRPolynomial<test_poly_t> poly(12);
    poly.set(2);
    LFSRVector<test_poly_t> vec(12,poly);
    std::cout << poly << std::endl;
    std::cout << vec << std::endl;
    vec.DoFeedback(vec);
    std::cout << vec << std::endl;
    vec.DoMultiShifts(poly,poly);
    std::cout << vec << std::endl;
}
