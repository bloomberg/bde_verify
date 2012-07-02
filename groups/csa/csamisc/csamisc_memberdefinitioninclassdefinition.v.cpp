// csamisc_memberdefinitioninclassdefinition.v.cpp                    -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csamisc_memberdefinitioninclassdefinition.v.hpp"
#ident "$Id$"

void cool::csamisc::memberdefinitioninclassdefinition_foo::g()
{
}

// -----------------------------------------------------------------------------

template <typename T>
inline void cool::csamisc::memberdefinitioninclassdefinition_bar<T>::f()
{
}

template class cool::csamisc::memberdefinitioninclassdefinition_bar<int>;

namespace cool
{
    namespace csamisc
    {
        namespace
        {
            void instantiate()
            {
                cool::csamisc::memberdefinitioninclassdefinition_bar<int> b;
            }
        }
    }
}

// ----------------------------------------------------------------------------

namespace cool
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

        void memberdefinitioninclassdefinition_update() {
            cool::csamisc::foobar u;
            cool::csamisc::foobar o(u);
            o.f();
        }
    }
}

