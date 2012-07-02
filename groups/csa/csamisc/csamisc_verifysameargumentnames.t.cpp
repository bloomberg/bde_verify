// csamisc_verifysameargumentnames.t.cpp                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_verifysameargumentnames.t.hpp"
#ident "$Id$"

namespace cool
{
    namespace csamisc
    {
        void verifysameargumentnames_f(int);
        void verifysameargumentnames_f(int);
        void verifysameargumentnames_f(int, int);
        void verifysameargumentnames_f(int) {}

        void verifysameargumentnames_g(int, int);
        void verifysameargumentnames_g(int a, int);
        void verifysameargumentnames_g(int, int b);
        void verifysameargumentnames_g(int a, int b);
        void verifysameargumentnames_g(int b, int a);

        void verifysameargumentnames_g(int, int) {}
    }
}
