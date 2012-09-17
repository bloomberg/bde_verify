// csamisc_thrownonstdexception.t.cpp                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_thrownonstdexception.t.hpp"
#include <bdes_ident.h>

#include <exception>

namespace cool
{
    namespace csamisc
    {
        namespace { typedef std::exception bar; }

        namespace
        {
            struct foo: bar
            {
                foo();
                foo(foo const&);
                ~foo() throw();
                void f();
            };
            foo::foo() {}
            foo::foo(foo const&) {}
            foo::~foo() throw() {}
            void foo::f()
            {
                throw 17;
                throw foo();
            }
        }

        namespace
        {
            foo f;
        }
    }
}
