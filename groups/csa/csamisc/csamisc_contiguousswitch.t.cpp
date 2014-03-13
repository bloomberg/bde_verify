// csamisc_contiguousswitch.t.cpp                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_contiguousswitch.t.hpp"
#include <bdes_ident.h>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            void f(int ac) {
                switch (ac) {
                case 5:
                    break;
                case 3:
                    break;
                }
            }
        }
    }
}

int main(int, char*[]);

int main(int ac, char* av[])
{
    switch (ac) {
    }

    switch (ac) { // starts with `default:` instead of `case 0:`
    default:
        ++ac;
        break;
    case 1:
        {
            ++ac;
            ++ac;
        }
        break;
    }

    switch (ac) { // doesn't start with `case 0:` and `case 0` in the middle
    case 1: {
        break; 
    }
    case 0: {
        ++ac;
        break;
    }
    default: ;
    }

    switch (ac) { // `default:` not at end
    case 0:
    default: break;
    case 1: break;
    }

    switch (ac) { // `default:` not at end
    case 0:
    case 2: break;
    default: break;
    case 1: break;
    }

    switch (ac) // missing default
    {
    case 0:
        switch (ac + 1) {
        case 10: break;
        case 12: break;
        }
    case 5: break;
    case 4:
    case 3: break;
    case 2: break;
    case 1: break;
    }

    switch (ac)
    {
    case 0:
    case 100: break;
    case 5: break;
    case 4: break;
    case 3: break;
    case 2: break;
    case 1: break;
    case -1: break;
    case -5: break;
    default: break;
    }

    return 0;
}
