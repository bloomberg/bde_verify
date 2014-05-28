// csamisc_memberdefinitioninclassdefinition.v.cpp                    -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_memberdefinitioninclassdefinition.v.hpp"
#include <bdes_ident.h>

void bde_verify::csamisc::memberdefinitioninclassdefinition_foo::g()
{
}

// -----------------------------------------------------------------------------

template <typename T>
inline void bde_verify::csamisc::memberdefinitioninclassdefinition_bar<T>::f()
{
}

template class bde_verify::csamisc::memberdefinitioninclassdefinition_bar<int>;

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            void instantiate()
            {
                bde_verify::csamisc::memberdefinitioninclassdefinition_bar<int> b;
            }
        }
    }
}

// ----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csamisc
    {
        struct base {
            virtual ~base();
            virtual void f();
        };
        struct foobar: base {
            void f();
        };

        void operator-(base&) {
            bde_verify::csamisc::foobar u;
            bde_verify::csamisc::foobar o(u);
            o.f();
        }

        void f() {
            struct g {
                g() { }
                void foo() { }
            };
        }
    }
}

