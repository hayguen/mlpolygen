//=============================================================================
//  A main function for using and testing the PrimeFactorizer
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

#ifdef USING_GMP
#include <gmpxx.h>
#endif

#include "PrimeFactorizer.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <stdio.h>
#include <iostream>

//-----------------------------------------------------------------------------
void usage(const char* argv0)
//-----------------------------------------------------------------------------
{
    printf("usage: %s [-b][-?] [num] [num] [...]\n", argv0);
    printf(" computes and prints prime factors (with orders) for a provided integer\n");
#ifdef USING_GMP
    printf("   -b  use bignum library (GMP)\n");
#endif
    printf("   -?  this help\n");
    printf(" if no numbers are provided on the command line, program is interactive\n");
}


template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int PrintFactorsOf(const std::string& str)
//-----------------------------------------------------------------------------
{
    std::stringstream ss(str);
    uintT val = 0;
    bool extracted = !!(ss >> val);
    if (!extracted)
        return -1;
    while (!ss.eof()) {
        char c;
        if (ss >> c)
            return -1; // non-whitespace remained
    }
    PrimeFactorizer<uintT,fltT> pf(val);
    std::cout << val << " == " << pf << std::endl;
    return 0;
}

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int main_interactive()
//-----------------------------------------------------------------------------
{
    int err = 0;
    while (true) {
        std::cout << "Enter an unsigned integer: ";
        std::string line;
        if (!std::getline(std::cin, line))
            break;
        err += PrintFactorsOf<uintT,fltT>(line);
    }
    std::cout << std::endl;
    return -err;
}

template <typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int main_args(int argc, char* const argv[])
//-----------------------------------------------------------------------------
{
    int err = 0;
    for (int idx=0; idx<argc; idx++) {
        err += PrintFactorsOf<uintT,fltT>(argv[idx]);
    }
    return -err;
}

//-----------------------------------------------------------------------------
int main(int argc, char* const argv[])
//-----------------------------------------------------------------------------
{
    int bignum = 0;

#ifndef _MSC_VER
    int c;

    while ((c = getopt(argc, argv, "b?")) != -1) {
        switch (c) {
#ifdef USING_GMP
            case 'b':
                bignum = 1;
                break;
#endif
            case '?':
                usage(argv[0]);
                return 0;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    argc -= optind;
    argv += optind;
#else
    for (int argidx = 1; argidx < argc; ++argidx)
    {
        if (!strcmp(argv[argidx], "-?") || !strcmp(argv[argidx], "-h"))
        {
            usage(argv[0]);
            return 0;
        }
    }
#endif

    int interactive = !argc;
    
    if (interactive && !bignum) {
        return main_interactive<uintmax_t,long double>();
#ifdef USING_GMP
    } else if (interactive) {
        return main_interactive<mpz_class,mpf_class>();
#endif
    } else if (!bignum) {
        return main_args<uintmax_t,long double>(argc,argv);
#ifdef USING_GMP
    } else {
        return main_args<mpz_class,mpf_class>(argc,argv);
#endif
    }
}
