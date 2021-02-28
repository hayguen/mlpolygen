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

#ifndef MLPolyTester_h
#define MLPolyTester_h
#pragma once

#include "LFSRPolynomial.h"
#include "LFSRVector.h"
#include "PrimeFactorizer.h"

#include <stdint.h>

//-----------------------------------------------------------------------------
class MLPolyTesterBase {
  public:
    MLPolyTesterBase(unsigned verbosity_=1): verbosity(verbosity_) {}
  protected:
    int dbprintf(unsigned level, const char *fmt, ...) const;
    unsigned verbosity;
};


template<typename poly_t=default_poly_t, typename uintT=uintmax_t, typename fltT=long double>
//-----------------------------------------------------------------------------
class MLPolyTester : protected MLPolyTesterBase {
  public:
    MLPolyTester(unsigned order, unsigned verbosity=1);
    
    int TestPolynomial(const poly_t& poly); // see implementation for return values

  protected:
    unsigned order;
    std::vector<poly_t> shifts;
};


//=============================================================================
//  template implementation
//=============================================================================


template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
MLPolyTester<poly_t,uintT,fltT>::MLPolyTester(unsigned ord, unsigned verbsty)
//-----------------------------------------------------------------------------
:   MLPolyTesterBase(verbsty), order(ord)
{
    dbprintf(3, "entering %s\n", __PRETTY_FUNCTION__);

    dbprintf(2, "Finding prime factors for 2**%u-1\n", order);
    uintT maxLen = ((uintT(1))<<ord)-uintT(1);
    PrimeFactorizer<uintT,fltT> factorizer(maxLen);
    unsigned numFactors = factorizer.Primes().size();
    dbprintf(2, "Found %d unique prime factors\n", numFactors);
    
    if (3<=verbosity) {
        std::cerr << "2**" << ord << "-1 = ";
        std::cerr << maxLen << " = " << factorizer << std::endl;
    }
    
    // compute the shifts, which are maxLen / factor[i]
    for (unsigned i=0; i<numFactors; i++) {
        uintT newUint = maxLen / factorizer.Primes()[i];
//      poly_t newPoly( newUint );
        poly_t newPoly;
        // to get from an arbitrary integer to a poly, loop over the bits
        for (unsigned bit=0; bit<order; bit++) {
            bool newBit = (newUint&(uintT(1)<<bit))!=0;
            newPoly.set(bit,newBit);
        }
        shifts.push_back( newPoly );
    }
}

template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int MLPolyTester<poly_t,uintT,fltT>::TestPolynomial(const poly_t& poly)
//  returns 0 if the polynomial is maximum length
//         -1 if order-1 self-feedbacks returns to the initial polynomial
//         -2 if order self-feedbacks does *not* return to the initial polynomial
//         -3 if polynomial failed the factors test
//-----------------------------------------------------------------------------
{
    LFSRVector<poly_t> theVec(order,poly);
    poly_t initialValue = theVec[0];

    for (unsigned i=0; i < order-1; i++) {
        theVec.DoFeedback(theVec);
        if (theVec[0] == initialValue) return -1;
    }
    // on the orderth case, we should return to the initial value
    theVec.DoFeedback(theVec);
    if (theVec[0] != initialValue) return -2;
    
    // passes preliminary test, now check the factors
    if (shifts.size() > 1) {
        poly_t result;
        result[order-1] = 1;
        for (int k=0; k<shifts.size(); k++) {
            theVec.DoMultiShifts(poly_t(shifts[k]), poly);
            if (theVec[0] == result) {
                return -3;
            }
        }
    }

    // passed all tests
    return 0;
}

#endif
