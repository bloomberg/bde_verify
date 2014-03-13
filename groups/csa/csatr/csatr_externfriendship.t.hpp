// csatr_externfriendship.t.hpp                                       -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSATR_EXTERNFRIENDSHIP)
#define INCLUDED_CSATR_EXTERNFRIENDSHIP 1
#ident "$Id$"

namespace bde_verify
{
    namespace csatr
    {
        class BadExtern
        {
        public:
            void f() const;
            class Nested;
            template <typename T> void g(T);
        };

        void operator+ (BadExtern const&);
        template <typename T>
        void operator+ (BadExtern const&, T);

        template <typename>
        class BadPackageTemplate
        {
        };
    }
}

#endif
