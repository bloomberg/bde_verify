// csamisc_memberdefinitioninclassdefinition.v.hpp                    -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSAMISC_MEMBERDEFINITIONINCLASSDEFINITION)
#define INCLUDED_CSAMISC_MEMBERDEFINITIONINCLASSDEFINITION 1
#ident "$Id$"

namespace cool
{
    namespace csamisc
    {
        struct memberdefinitioninclassdefinition_foo
        {
            void f() {}
            void g();
        };

        template <class T>
        struct memberdefinitioninclassdefinition_bar
        {
            void f();
            void g() {}
        };

        struct base;
        struct foobar;
        void operator+(base&);
        void operator-(base&);
    }
}
#endif
