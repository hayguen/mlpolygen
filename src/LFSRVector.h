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

#ifndef LFSRVector_h
#define LFSRVector_h
#pragma once

#include "LFSRPolynomial.h"
#include <vector>


template<typename poly_t=default_poly_t>
//-----------------------------------------------------------------------------
class LFSRVector {

  public:
    LFSRVector(unsigned order);
    LFSRVector(unsigned order, const poly_t& p);

    void Init(const poly_t& p); // in-place initialization

    void print(std::ostream& os) const;
    
    void DoFeedback(const LFSRVector& fbvec);
    void DoMultiShifts(const poly_t& numShifts, const poly_t& poly);

    operator const poly_t&(void) const { return vec[0]; }
    poly_t& operator[](unsigned n) { return vec[n]; }
    const poly_t& operator[](unsigned n) const { return vec[n]; }

  protected:
    std::vector<poly_t> vec;
};


template<typename poly_t>
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const LFSRVector<poly_t>& vec)
//-----------------------------------------------------------------------------
{
    vec.print(os);
    return os;
}


//=============================================================================
//  template implementation
//=============================================================================


template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRVector<poly_t>::LFSRVector(unsigned order)
//-----------------------------------------------------------------------------
:   vec(order,0)
{
}

template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRVector<poly_t>::LFSRVector(unsigned order, const poly_t& poly)
//  set the vector to its initial values based on the poly
//-----------------------------------------------------------------------------
:   vec(order,0)
{
    Init(poly);
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRVector<poly_t>::Init(const poly_t& poly)
//  in-place initialization
//-----------------------------------------------------------------------------
{
    unsigned order = vec.size();
    for (unsigned i=0; i<order; i++) {
        vec[i] = 0;
    }

    for (unsigned i=0; i<order-1; i++) {
        vec[i][order-2-i] = 1;
    }
    vec[order-1][order-1] = 1;

    for (unsigned i=0; i<order; i++) {
        if (poly[i]) {
            vec[i][order-1] = 1;
        }
    }
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRVector<poly_t>::print(std::ostream& os) const
//-----------------------------------------------------------------------------
{
    unsigned order = vec.size();
    os << "[ " << LFSRPolynomial<poly_t>(order, vec[0]);
    for (unsigned i=1; i<order; i++) {
        os << ", " << LFSRPolynomial<poly_t>(order, vec[i]);
    }
    os << " ]";
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRVector<poly_t>::DoFeedback(const LFSRVector& fbvec)
//  update this vector, feeding back with fbvec
//-----------------------------------------------------------------------------
{
    int order = vec.size();
    LFSRVector<poly_t> result(order);

    for (int i=0; i<order; i++) {
        for (int j=0; j<order; j++) {
            if ( fbvec[i][order-1-j] ) {
                result[i] ^= vec[j];
            }
        }
    }
    *this = result;
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRVector<poly_t>::DoMultiShifts(const poly_t& numShifts, const poly_t& poly)
//-----------------------------------------------------------------------------
{
    unsigned order = vec.size();
    Init(poly);
    LFSRVector fbvec = *this;

    unsigned i;
    for (i=order-1; i<order; i--) {
        if (numShifts[i]) break;
    }
    i--;
    
    for (; i<order; i--) {
        fbvec = *this;
        DoFeedback(fbvec);
        if (numShifts[i]) {
            fbvec.Init(poly);
            DoFeedback(fbvec);
        }
    }
}

#endif
