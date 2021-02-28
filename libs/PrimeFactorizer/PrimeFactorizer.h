//=============================================================================
//  A simple class for factorizing integers into prime factors
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

#ifndef PrimeFactorizer_h
#define PrimeFactorizer_h
#pragma once

#include <stdint.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>


template <typename uintT=uintmax_t, typename fltT=long double>
//-----------------------------------------------------------------------------
class PrimeFactorizer {
//  integer type is uintT, fltT is needed for std::sqrt()
//-----------------------------------------------------------------------------
  public:
    PrimeFactorizer(uintT num);

    const std::vector<uintT>& Primes(void) const { return primes; }
    const std::vector<uintT>& Orders(void) const { return orders; }

    void print(std::ostream& os) const;

  private:
    std::vector<uintT> primes;
    std::vector<uintT> orders;
    
    void AddPrimeFactor(const uintT& v);
    static uintT SquareRoot(const uintT& v);
};

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
std::ostream& operator<< (std::ostream& os, PrimeFactorizer<uintT,fltT> pf)
//-----------------------------------------------------------------------------
{
    pf.print(os);
    return os;
}


//=============================================================================
//  template implementation
//=============================================================================

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
PrimeFactorizer<uintT,fltT>::PrimeFactorizer(uintT num)
//-----------------------------------------------------------------------------
{
    if (num==0) return;
    if (num==1) {
        AddPrimeFactor(1);
        return;
    }

    // elimimate all factors of 2
    while ((num & uintT(1))==0) {
        AddPrimeFactor(2);
        num >>= 1;
        if (num==1) return;
    }
    
    uintT lastPrime = SquareRoot(num);
    uintT primeCandidate = 3;
    bool doubleSkip = 0; // to remove 3*n from the candidates
    while (primeCandidate<=lastPrime) {
        if ( (num % primeCandidate)==0 ) {
            //std::cout << "# " << num << " = ";
            num /= primeCandidate;
            lastPrime = SquareRoot(num);
            //std::cout << primeCandidate << " * " << num << std::endl;
            AddPrimeFactor(primeCandidate);
        } else {
            // not a multiple, move on
            primeCandidate += 2;
            // this could be smarter (prime wheel) to be faster
#if 1
            // currently skips multipes of 2 and 3
            if (primeCandidate<=7)
                doubleSkip = 0;
            if (doubleSkip)
                primeCandidate += 2;
            doubleSkip = !doubleSkip;
#endif
        }
    }
    AddPrimeFactor(num);
}

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
void PrimeFactorizer<uintT,fltT>::print(std::ostream& os) const
//-----------------------------------------------------------------------------
{
    if (!primes.size()) {
        os << "0";
    }
    for (unsigned pidx=0; pidx<primes.size(); pidx++) {
        if (pidx)
            os << " * ";
        if (orders[pidx]<2) {
            os << primes[pidx];
        } else {
            os << primes[pidx] << "**" << orders[pidx];
        }
    }
}

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
void PrimeFactorizer<uintT,fltT>::AddPrimeFactor(const uintT& v)
//-----------------------------------------------------------------------------
{
    unsigned size = primes.size();
    if (!size || primes[size-1]<v) {
        primes.push_back(v);
        orders.push_back(1);
    } else if (primes[size-1]==v) {
        orders[size-1]++;
    } else {
        std::stringstream ss;
        ss << "error: prime numbers decreased from " << primes[size-1];
        ss << " to " << v;
        throw std::runtime_error(ss.str());
    }
}

#include <cmath>

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
uintT PrimeFactorizer<uintT,fltT>::SquareRoot(const uintT& v)
//-----------------------------------------------------------------------------
{
    fltT vv = v;
    vv = std::sqrt(vv);
    return uintT(vv);
}

#endif
