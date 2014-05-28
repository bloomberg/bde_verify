// csamisc_arrayinitialization.t.cpp                                  -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_arrayinitialization.t.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            struct s
            {
                s(int = 0);                                         // IMPLICIT
            };
            s::s(int) {}
            
            struct t
            {
                t();
                t(int);                                             // IMPLICIT
            };
            t::t() {}
            t::t(int) {}
        }
        namespace { struct p { int a; int b; }; }
    }
}

// -----------------------------------------------------------------------------

int main(int ac, char* av[])
{
    bde_verify::csamisc::p pair = { 0 };
    bde_verify::csamisc::s   array00[5];
    int array01[] = { ' ' };
    int array02[1] = { ' ' };
    int array03[5] = { ' ' };
    int array04[5] = { };
    int array05[5] = { 0 };
    int array06[5] = { ' ', 0 };
    int array07[5] = { int() };
    int array08[5] = { ' ', int() };
    int array09[5] = { ' ', int(' ') };
    int array10[5] = { ' ', int('\0') };
    int array11[5] = { ' ', '\0' };

    bde_verify::csamisc::s array12[5] = { };
    bde_verify::csamisc::s array13[5] = { bde_verify::csamisc::s() };
    bde_verify::csamisc::s array14[5] = { bde_verify::csamisc::s(1) };
    bde_verify::csamisc::s array15[5] = { bde_verify::csamisc::s(1), bde_verify::csamisc::s() };

    bde_verify::csamisc::t array16[5] = {};
    bde_verify::csamisc::t array17[5] = { bde_verify::csamisc::t() };
    bde_verify::csamisc::t array18[5] = { bde_verify::csamisc::t(1) };
    bde_verify::csamisc::t array19[5] = { bde_verify::csamisc::t(1), bde_verify::csamisc::t() };

    int const  array20[] = { 0, 1 };
    char const array21[] = "foobar";
    char       array22[] = "foobar";
    char const array23[] = { "foobar" };
    char       array24[] = { "foobar" };
}
