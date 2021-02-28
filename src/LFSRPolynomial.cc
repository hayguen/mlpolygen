//=============================================================================
//  A class for a single polynomial with some properties and operations
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

#include "LFSRPolynomial.h"

int test_LFSRPolynomial()
{
    LFSRPolynomial<> poly(64);
    poly.set(16);

    std::cout << poly << std::endl;
    poly.printAsHex(std::cout);
    std::cout << std::endl;
    poly.printAsPoly(std::cout);
    std::cout << std::endl;
    
    LFSRPolynomial<> poly2(poly);
    LFSRPolynomial<> poly3(poly.SymmetricDual());
    std::cout << poly3 << std::endl;
    
    LFSRPolynomial< std::bitset<1024> > poly4(1024);

    LFSRPolynomial<> poly5("1111100000101111");
    std::cout << poly5 << std::endl;
    LFSRPolynomial<> poly6("1010101010101010101010101010101010101010101010101010101010101010");
    LFSRPolynomial<> poly8("1");
//  LFSRPolynomial<> poly9("0");

    LFSRPolynomial<> poly7(64,64);
}

