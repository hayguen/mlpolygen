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

#ifndef LFSRPolynomial_h
#define LFSRPolynomial_h
#pragma once

#include <bitset>
#include <iostream>


typedef std::bitset<64> default_poly_t;

template<typename poly_t=default_poly_t>
//-----------------------------------------------------------------------------
class LFSRPolynomial {
  public:
    LFSRPolynomial(unsigned order);
    LFSRPolynomial(unsigned order, const poly_t& p);
    LFSRPolynomial(const std::string& s); // figure out the order

    unsigned Order(void) const { return numBits; }

    unsigned NumBitsSet(void) const;        // the number of ones in the polynomial
    int IsAsymmetric(void) const;   // properties of symmetry about the middle
    LFSRPolynomial SymmetricDual(void) const; // return the symmetric dual

    void SetRandom(void);
    void SetMax(void);

    void printAsHex(std::ostream& os) const;
    void printAsPoly(std::ostream& os) const;

    operator const poly_t&(void) const { return poly; }
    
    bool operator[](int n) const { return poly[n]; }
    poly_t& set(int n, int val = 1) { return poly.set(n,val); }
    bool operator<(const LFSRPolynomial<poly_t>& lp) const;
    
    // iterator-like for iterating over potential candidates
    const LFSRPolynomial<poly_t>& next_candidate(void);
    bool end_candidate(void) const;
    
  protected:
    poly_t   poly;
    unsigned numBits;
};


template<typename poly_t>
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const LFSRPolynomial<poly_t>& poly)
//-----------------------------------------------------------------------------
{
    poly.printAsHex(os);
    return os;
}


//=============================================================================
//  template implementation
//=============================================================================

#include <assert.h>
#include <stdlib.h>

template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRPolynomial<poly_t>::LFSRPolynomial(unsigned order)
//-----------------------------------------------------------------------------
:   numBits(order)
{
    assert(order>0);
    assert(order<=poly.size());
    
    poly = poly_t(1)<<(order-1);
}

template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRPolynomial<poly_t>::LFSRPolynomial(unsigned order, const poly_t& p)
//-----------------------------------------------------------------------------
:   numBits(order), poly(p)
{
    assert(order>0);
    assert(order<=poly.size());
}

template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRPolynomial<poly_t>::LFSRPolynomial(const std::string& s)
//-----------------------------------------------------------------------------
:   numBits(0), poly(s)
{
    if (s.size()>poly.size()) {
        std::cerr << "string too large to fit in poly (" << std::dec;
        std::cerr << s.size() << ">" << poly.size() << ")" << std::endl;
        
    }
    assert(s.size()<=poly.size());

    // find order from highest set bit
    numBits=poly.size();
    while (numBits && !poly[numBits-1])
        numBits--;

    assert(numBits>0);
    assert(numBits<=poly.size());
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRPolynomial<poly_t>::printAsHex(std::ostream& os) const
//-----------------------------------------------------------------------------
{
    os << std::hex;
    for (int i=(numBits-1)&(-4); i>=0; i-=4) {
        int nibble = poly[i] + poly[i+1]*2 + poly[i+2]*4 + poly[i+3]*8;
        os << nibble;
    }
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRPolynomial<poly_t>::printAsPoly(std::ostream& os) const
//-----------------------------------------------------------------------------
{
    os << std::dec << "0";
    for (unsigned i=0; i<numBits; i++) {
        if ( poly[i] )
            os << "," << i+1;
    }
}

template<typename poly_t>
//-----------------------------------------------------------------------------
unsigned LFSRPolynomial<poly_t>::NumBitsSet(void) const
//  returns the number of bits set to 1 in this polynomial
//-----------------------------------------------------------------------------
{
    unsigned result = 0;
    for (unsigned i=0; i<numBits; i++) {
        if ( poly[i] )
            result++;
    }
    return result;
}

template<typename poly_t>
//-----------------------------------------------------------------------------
int LFSRPolynomial<poly_t>::IsAsymmetric(void) const
//  0 indicates this polynomial is symetric about the middle
//  non-zero indicates it is asymmetric
//  -1 indicates it has extra bit(s) set in LSBs
//  1 indicates it has extra bit(s) set in MSBs
//-----------------------------------------------------------------------------
{   
    for (int i=1; i <= numBits/2; i++) {
        // if symmetric, bits are equal
        if ( poly[i-1] == poly[numBits-i-1] ) continue;
        else return poly[i-1] ? -1 : 1;
    }
    return 0;
}

template<typename poly_t>
//-----------------------------------------------------------------------------
LFSRPolynomial<poly_t> LFSRPolynomial<poly_t>::SymmetricDual(void) const
//  return the symmetric dual of this polynomial
//-----------------------------------------------------------------------------
{
    LFSRPolynomial<poly_t> result(numBits);
    
    for (int i=1; i <= numBits/2; i++) {
        result.poly[i-1] = poly[numBits-i-1];
        result.poly[numBits-i-1] = poly[i-1];
    }
    
    return result;
}

#include <sys/time.h>

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRPolynomial<poly_t>::SetRandom(void)
//-----------------------------------------------------------------------------
{
    timeval tv;
    gettimeofday(&tv,0);
    unsigned seed = tv.tv_usec;
    srandom(seed);
    poly = poly_t(1)<<(numBits-1);
    for (unsigned i=0; i<numBits-1; i++) {
        poly.set(i,random()&1);
    }
}

template<typename poly_t>
//-----------------------------------------------------------------------------
void LFSRPolynomial<poly_t>::SetMax(void)
//-----------------------------------------------------------------------------
{
    poly = poly_t(1)<<(numBits-1);
    for (unsigned i=0; i<numBits-1; i++) {
        poly.set(i,1);
    }
}

template<typename poly_t>
//-----------------------------------------------------------------------------
bool LFSRPolynomial<poly_t>::operator<(const LFSRPolynomial<poly_t>& lp) const
//-----------------------------------------------------------------------------
// true if this < lp
{
    if (numBits != lp.numBits)
        return numBits < lp.numBits;
    unsigned i=numBits;
    for (; i; i--) {
        bool A = operator[](i-1);
        bool B = lp[i-1];
        if (A != B)
            return A<B;
    }
    return false;
}

template<typename poly_t>
//-----------------------------------------------------------------------------
const LFSRPolynomial<poly_t>& LFSRPolynomial<poly_t>::next_candidate(void)
//  candidates have an even number of bits set
//-----------------------------------------------------------------------------
{
    poly_t tmp(0);
    if (tmp == poly) return *this;
    
    do {
        unsigned i = 0;
        while (poly[i]) {
            poly.set(i,0);
            i++;
            if (i >= numBits) { // end case
                poly = 0;
                return *this;
            }
        }
        poly.set(i,1);
    } while (NumBitsSet() & 1);

    return *this;
}

template<typename poly_t>
//-----------------------------------------------------------------------------
bool LFSRPolynomial<poly_t>::end_candidate(void) const
//-----------------------------------------------------------------------------
{
    poly_t tmp(0);
    return tmp == poly;
}

#endif
