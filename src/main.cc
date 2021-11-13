//=============================================================================
//  The main program for MLPolyGen
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

#include "MLPolyTester.h"

#include <cargs.h>

#include <deque>
#include <stdio.h>
#include <assert.h>
#include <iomanip>

#ifndef MLPOLYGEN_VERSION
#define MLPOLYGEN_VERSION "UNKNOWN"
#endif

static struct cag_option options[] = {
#ifdef USING_GMP
    {'b', "b", NULL, NULL, "use bignum library (GMP), may be auto-selected by order" },
#endif
    {'p', "p", NULL, NULL, "use symmetric pairs (faster but unsorted output)"},
    {'c', "c", NULL, NULL, "print polynomials amended with count of taps"},
    {'2', "2", NULL, NULL, "search for polynomials with only 2 taps for specified order"},

    {'u', "u", NULL, "shiftUp", "for option '-f': first enabled tap. default: 0"},
    {'f', "f", NULL, "bits",
        "bruteForce: how many taps(bits) should be tested sequenctially:\n"
        "\tall possible combinations for taps u .. u+f-1 are tested\n"
        "\tdefault: 0 for off.limitation : all taps need to fit into 64 Bits!"},
    {'m', "m", NULL, "int",
        "print only polynomials with number of taps <= this value\n"
        "\tdefault is -1, to print all polynomials"},
    {'s', "s", NULL, "start",
        "start with specified polynomial (order is computed, not required)"},
    {'e', "e", NULL, "end",
        "end with specified polynomial (order is computed, not required)"},
    {'n', "n", NULL, "number",
        "stop after specified number of ML polynomials"},

    {'r', "r", NULL, NULL, "compute random ML polys (may not be unique, can use with -n)"},
    {'t', "t", NULL, "poly",
        "test the specified polynomial (order is computed, not required)"},

    {'v', "v", NULL, NULL, "increase verbosity"},
    {'h', "h?", "help", NULL, "this help"},
};



//-----------------------------------------------------------------------------
void usage(const char* argv0)
//-----------------------------------------------------------------------------
{
    printf("usage: %s [options] order\n", argv0);
    printf("  generates/tests polynomials for maximal length (ML) for an LFSR\n");
    printf("\noptions:\n");
    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
    printf("\narguments:\n");
    printf("  order is the unsigned integer order of the polynomials to compute\n");
#ifndef USING_GMP
    printf("Compiled without bignum (GMP) support\n");
#endif
    printf("mlpolygen version %s\n", MLPOLYGEN_VERSION);
}


template <typename uintT>
//-----------------------------------------------------------------------------
int GetUintAsBinaryStr(const char str[], uintT & val, std::string& ostr)
//  because that's what bitset wants
//-----------------------------------------------------------------------------
{
    std::stringstream ss(str);

    // leading 0x indicates hex, leading 0 indicates oct
    if (ss.peek() == '0') {
        char c;
        ss.get(c);
        c = ss.peek();
        if (c == 'x' || c == 'X') {
            ss.get(c);
            ss.setf(std::ios::hex, std::ios::basefield);
        } else {
            ss.setf(std::ios::oct, std::ios::basefield);
        }
    }
    
    // get the value
    bool extracted = !!(ss >> val);
    if (!extracted)
        return -1;
    while (!ss.eof()) {
        char c;
        if (ss >> c)
            return -1; // non-whitespace remained
    }
    
    // now write it to ostr as a binary string 
    uintT val2 = val;
    std::deque<char> bdeq;
    while (val2!=0) {
        bdeq.push_front( (val2&uintT(1))!=0 ? '1' : '0' );
        val2 /= 2;
    }
    
    while (bdeq.size()) {
        ostr += bdeq[0];
        bdeq.pop_front();
    }
    
    return 0;
}

template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int TestSinglePolynomial(const char str[], int verbosity=0)
//-----------------------------------------------------------------------------
{
    std::string bstr;
    uintT val;
    int result = GetUintAsBinaryStr<uintT>(str,val,bstr);
    if (result) {
        std::cerr << "Error converting to uint" << sizeof(uintT)*8;
        std::cerr << ": " << str << std::endl;
        return result;
    }

    LFSRPolynomial<poly_t> poly(bstr.c_str());
    MLPolyTester<poly_t,uintT,fltT> polyTester(poly.Order(),verbosity);
    result = polyTester.TestPolynomial(poly);

    std::cout << std::hex << std::setiosflags( std::ios::showbase );
    std::cout << val << " is ";
    if (result)
        std::cout << "NOT ";
    std::cout << "maximal length";
    std::cout << " for order " << std::dec << poly.Order() << std::endl;
    std::cout << std::resetiosflags(std::ios::showbase | std::ios::basefield);
    return result;
}

template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
unsigned FindTwoTapPolynomials(unsigned order, int verbosity=0)
//-----------------------------------------------------------------------------
{
    LFSRPolynomial<poly_t> poly(order);
    MLPolyTester<poly_t,uintT,fltT> polyTester(poly.Order(),verbosity);
    unsigned n_results = 0;
    int result;

    std::cout << std::hex << std::resetiosflags(std::ios::showbase | std::ios::basefield);
    for (unsigned k = 0; k < order -1; ++k)
    {
        poly.Clear();
        poly.set(order -1, 1);
        poly.set(k, 1);

        result = polyTester.TestPolynomial(poly);
        if (result)
            continue;
        ++n_results;
        std::cout << std::dec << n_results << ": 0x" << poly;
        if (verbosity >= 1) {
            const unsigned n_taps_set = poly.NumBitsSet();
            std::cout << "\t# " << std::dec << n_taps_set;
            if (verbosity >= 2)
                std::cout << ": 0," << k+1 << "," << order;
        }
        std::cout << std::endl;
    }
    std::cout << std::dec;
    return n_results;
}

template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
unsigned BruteForceFindPolynomials(int shiftUp, int bruteForceNumBits, unsigned order, int verbosity=0)
//-----------------------------------------------------------------------------
{
    if (order > 64) {
        std::cerr << "order = " << order
            << ": only values <= 64 supported!" << std::endl;
        return 0;
    }
    if (shiftUp < 0) {
        std::cerr << "shiftUp = " << shiftUp
            << ": only values >= 0 supported!" << std::endl;
        return 0;
    }
    if (bruteForceNumBits <= 0) {
        std::cerr << "bruteForceNumBits = " << bruteForceNumBits
            << ": only values > 0 supported!" << std::endl;
        return 0;
    }
    if (shiftUp+bruteForceNumBits >= order) {
        std::cerr << "shiftUp + bruteForceNumBits = " << shiftUp + bruteForceNumBits
            << ": only values < 'order' supported!" << std::endl;
        return 0;
    }

    LFSRPolynomial<poly_t> poly(order);
    MLPolyTester<poly_t,uintT,fltT> polyTester(poly.Order(),verbosity);
    unsigned n_results = 0;
    int result;

    std::cout << std::hex << std::resetiosflags(std::ios::showbase | std::ios::basefield);
    uint64_t forceNmax = 0;
    for (unsigned k = 0; k < bruteForceNumBits; ++k)
        forceNmax |= (uint64_t(1) << k);
    for (uint64_t comb = 1; comb <= forceNmax; ++comb)
    {
        poly.Clear();
        poly.set(order -1, 1);
        for (unsigned k = 0; k < bruteForceNumBits; ++k) {
            if ( (comb >> k) & 1 ) {
                assert( shiftUp + k != order -1 );
                poly.set(shiftUp + k, 1);
            }
        }

        result = polyTester.TestPolynomial(poly);
        if (result)
            continue;
        ++n_results;
        std::cout << std::dec << n_results << ": 0x" << poly;
        if (verbosity >= 1) {
            const unsigned n_taps_set = poly.NumBitsSet();
            std::cout << "\t# " << std::dec << n_taps_set;
            if (verbosity >= 1) {
                std::cout << ": 0";
                for (unsigned k = 0; k < order; ++k) {
                    if (poly[k])
                        std::cout << "," << k+1;
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::dec;
    return n_results;
}


template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int GenerateRandomPolys(unsigned long order, unsigned long numRands, int verbosity=0)
//-----------------------------------------------------------------------------
{
    MLPolyTester<poly_t,uintT,fltT> polyTester(order,verbosity);
    while (numRands) {
        LFSRPolynomial<poly_t> poly(order);
        poly.SetRandom();
        if (1<=verbosity) {
            std::cerr << "Random poly: " << poly << std::endl;
        }
        while (1) {
            int result = polyTester.TestPolynomial(poly);
            if (!result) {
                std::cout << poly << std::endl;
                numRands--;
                break;
            }
            poly.next_candidate();
            if (poly.end_candidate()) {
                poly = LFSRPolynomial<poly_t>(order);
            }
            if (2<=verbosity) {
                std::cerr << "Next poly: " << poly << std::endl;
            }
        }
    }
    return 0;
}

template<typename poly_t, typename uintT, typename fltT>
//-----------------------------------------------------------------------------
int GeneratePolySequence(unsigned long order, const char* startVal, const char* endVal, unsigned long numPolys, bool inPairs, int verbosity=0, bool printCountTaps =false, int maximum_taps =-1)
//-----------------------------------------------------------------------------
{
    LFSRPolynomial<poly_t> poly(order?order:1); // use a dummy when !order
    if (startVal) {
        std::string bstr;
        uintT val;
        int result = GetUintAsBinaryStr<uintT>(startVal,val,bstr);
        if (result) {
            std::cerr << "Error converting to uint" << sizeof(uintT)*8;
            std::cerr << ": " << startVal << std::endl;
            return result;
        }
        poly = LFSRPolynomial<poly_t>(bstr);
    }
    LFSRPolynomial<poly_t> endPoly(1);
    if (endVal) {
        std::string bstr;
        uintT val;
        int result = GetUintAsBinaryStr<uintT>(endVal,val,bstr);
        if (result) {
            std::cerr << "Error converting to uint" << sizeof(uintT)*8;
            std::cerr << ": " << endVal << std::endl;
            return result;
        }
        endPoly = LFSRPolynomial<poly_t>(bstr);
    }
    if (startVal && endVal) {
        if (poly.Order() != endPoly.Order()) {
            std::cerr << "Error: order for start and end values must match (";
            std::cerr << poly.Order() << " vs " << endPoly.Order() << ")" << std::endl;
            return -1;
        }
    }
    if (!startVal && endVal) {
    	poly = LFSRPolynomial<poly_t>(endPoly.Order());
    }
    if ((startVal || endVal) && order && (poly.Order() != order)) {
        std::cerr << "Order multiply specified as " << poly.Order() << " and " << order;
        std::cerr << ", using " << poly.Order() << " from the start value" << std::endl;
    }
    order = poly.Order();

    if (1<=verbosity && !numPolys) {
        std::cerr << "Generating maximal length polynomials of order " << order << "," << std::endl;
        LFSRPolynomial<poly_t> opoly = poly;
        std::cerr << "  from 0x" << opoly;
        if (endVal) opoly = endPoly;
        else opoly.SetMax();
        std::cerr << " to 0x" << opoly;
        if (inPairs)
            std::cerr << " (in pairs)";
        std::cerr << std::endl;
    }

    MLPolyTester<poly_t,uintT,fltT> polyTester(order,verbosity);
    unsigned long polysFound = 0;
    while (1) {
        while (inPairs && (poly.IsAsymmetric()==1) && !poly.end_candidate()) {
            poly.next_candidate();
        }
        if (poly.end_candidate()) { // have we reached the last possible candidate?
            break;
        }
        if (numPolys && polysFound>=numPolys) { // have we already found enough?
            break;
        }
        if (endVal && endPoly<poly) { // have we passed a spec's endVal?
            break;
        }
        if (2<=verbosity) {
            std::cerr << "candidate: " << poly << std::endl;
        }
        int result = polyTester.TestPolynomial(poly);
        if (!result) {
            const unsigned n_taps_set = poly.NumBitsSet();
            if (maximum_taps == -1 || n_taps_set <= maximum_taps) {
                if (!printCountTaps)
                    std::cout << poly << std::endl;
                else
                    std::cout << poly << "\t# " << std::dec << n_taps_set << std::endl;
                polysFound++;
                if (inPairs && (poly.IsAsymmetric()==-1)) { // is asymmetric and has more lower bits
                    std::cout << poly.SymmetricDual() << std::endl;
                    polysFound++;
                }
            }
        }
        poly.next_candidate();
    }
    return 0;
}

typedef default_poly_t reg_poly_t;
typedef uintmax_t reg_uint_t;
typedef long double reg_float_t;

#ifdef USING_GMP
typedef std::bitset<1024> big_poly_t;
typedef mpz_class big_uint_t;
typedef mpf_class big_float_t;
#endif



//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
    const char* argv0 = argv[0];
    int bignum = 0;
    int verbosity = 0;
    int result = 0;
    int tested = 0;
    int maximum_taps = -1;
    int shiftUp = 0;
    int bruteForceNumBits = 0;
    bool inPairs = 0;
    bool doRandom = 0;
    bool printCountTaps = false;
    bool findTwoTaps = false;
    const char* startVal = 0;
    const char* endVal = 0;
    unsigned long numPolys = 0;

    cag_option_context context;
    cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context)) {
        const char* optarg;
        char* endp;
        switch (cag_option_get(&context)) {
            case 't':
                optarg = cag_option_get_value(&context);
                if (!bignum) {
                    result += TestSinglePolynomial<reg_poly_t,reg_uint_t,reg_float_t>(optarg, verbosity);
#ifdef USING_GMP
                } else {
                    result += TestSinglePolynomial<big_poly_t,big_uint_t,big_float_t>(optarg, verbosity);
#endif
                }
                tested++;
                break;
            case 'r':
                doRandom = 1;
                break;
            case 'c':
                printCountTaps = true;
                break;
            case 'm':
                maximum_taps = atoi(cag_option_get_value(&context));
                break;
            case 'u':
                shiftUp = atoi(cag_option_get_value(&context));
                break;
            case 'f':
                bruteForceNumBits = atoi(cag_option_get_value(&context));
                break;
            case '2':
                findTwoTaps = true;
                break;
            case 's':
                startVal = cag_option_get_value(&context);
                break;
            case 'e':
                endVal = cag_option_get_value(&context);
                break;
            case 'n':
                optarg = cag_option_get_value(&context);
                numPolys = strtoul(optarg,&endp,0);
                if (endp[0]) {
                    std::cerr << "Error converting to uint: " << optarg << std::endl;
                    return -1;
                }
                break;
#ifdef USING_GMP
            case 'b':
                bignum = 1;
                break;
#endif
            case 'p':
                inPairs = 1;
                break;
            case 'v':
                verbosity++;
                break;
            case '?':
            case 'h':
                usage(argv[0]);
                return 0;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    const int optind = cag_option_get_index(&context);
    argc -= optind;
    argv += optind;

    if (argc>1) {
        std::cerr << "Error: too many arguments" << std::endl;
        usage(argv0);
        return -1;
    }
    if (tested) {
        if (argc) {
            std::cerr << "Note: option -t excludes the argument order" << std::endl;
        }
        return result;
    }
    unsigned long order = 0;
    if (argc) {
        char* endp;
        order = strtoul(argv[0],&endp,0);
        if (endp[0]) {
            std::cerr << "Error converting to uint: " << argv[0] << std::endl;
            return -1;
        }
    } else if (!startVal && !endVal) {
        std::cerr << "Error: not enough arguments" << std::endl;
        usage(argv0);
        return -1;
    }
    if (order>sizeof(reg_poly_t)*8 && !bignum) {
        std::cerr << "Maximum order (without bignum/GMP) is " << sizeof(reg_poly_t)*8;
#ifdef USING_GMP
        std::cerr << ", setting bignum" << std::endl;
        bignum = 1;
    }
    if (order>sizeof(big_poly_t)*8 && !bignum) {
        std::cerr << "Maximum order is " << sizeof(big_poly_t)*8;
#endif
        std::cerr << std::endl;
        return -1;
    }

    if (findTwoTaps) {
        unsigned n_results = 0;
        if (!bignum)
            n_results = FindTwoTapPolynomials<reg_poly_t,reg_uint_t,reg_float_t>(order, verbosity);
#ifdef USING_GMP
        else
            n_results += FindTwoTapPolynomials<big_poly_t,big_uint_t,big_float_t>(order, verbosity);
#endif
        std::cout << "found " << std::dec << n_results << " polynomials with 2 taps, order "
            << order << " and maximal length" << std::endl;
        return 0;
    }

    if (bruteForceNumBits) {
        unsigned n_results = 0;
        if (!bignum)
            n_results = BruteForceFindPolynomials<reg_poly_t,reg_uint_t,reg_float_t>(shiftUp, bruteForceNumBits, order, verbosity);
#ifdef USING_GMP
        else
            n_results += BruteForceFindPolynomials<big_poly_t,big_uint_t,big_float_t>(shiftUp, bruteForceNumBits, order, verbosity);
#endif
        std::cout << "found " << std::dec << n_results << " polynomials with all taps - except top - in "
            << shiftUp << " .. " << shiftUp + bruteForceNumBits
            << " of order " << order << " and maximal length" << std::endl;
        return 0;
    }

    if (doRandom) {
        if (inPairs || startVal || endVal)
            std::cerr << "Note: option -r excludes these options: -p -s -e " << std::endl;
        if (!numPolys) numPolys = 1;
        if (order<=sizeof(reg_poly_t)*8 && !bignum) {
            return GenerateRandomPolys<reg_poly_t,reg_uint_t,reg_float_t>(order,numPolys,verbosity);
#ifdef USING_GMP
        } else {
            return GenerateRandomPolys<big_poly_t,big_uint_t,big_float_t>(order,numPolys,verbosity);
#endif
        }
    }
    
    if (!bignum) {
        return GeneratePolySequence<reg_poly_t,reg_uint_t,reg_float_t>(order,startVal,endVal,numPolys,inPairs,verbosity,printCountTaps,maximum_taps);
#ifdef USING_GMP
    } else {
        return GeneratePolySequence<big_poly_t,big_uint_t,big_float_t>(order,startVal,endVal,numPolys,inPairs,verbosity,printCountTaps,maximum_taps);
#endif
    }
}
