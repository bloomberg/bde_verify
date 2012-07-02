// csatr_friendship.t.hpp                                             -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSATR_FRIENDSHIP)
#define INCLUDED_CSATR_FRIENDSHIP 1
#ident "$Id$"

#include "csatr_externfriendship.t.hpp"

namespace cool
{
    namespace csamisc
    {
        class BadGroup;
    }

    namespace csatr
    {
        class BadPackage;
        template <typename> class friendship_GoodTemplate;
        class friendship_GoodDeclared;
        void operator+ (friendship_GoodDeclared const&);
        template <typename T>
        void operator+ (friendship_GoodDeclared const&, T);

        class friendship_Component
        {
        public:
            friend class cool::csamisc::BadGroup;
            friend class cool::csatr::BadPackage;
            friend class cool::csatr::BadExtern;
            friend void cool::csatr::BadExtern::f() const;
            friend class cool::csatr::BadExtern::Nested;
            friend class cool::csatr::friendship_GoodDeclared;
            friend class GoodLocal;
            template <typename T> friend class BadPackageTemplate;
            template <typename T> friend class GoodTemplate;
            friend void cool::csatr::operator+ (BadExtern const&);
            template <typename T>
            friend void cool::csatr::operator+ (BadExtern const&, T);

            friend void cool::csatr::operator+ (friendship_GoodDeclared const&);
            template <typename T>
            friend void cool::csatr::operator+ (friendship_GoodDeclared const&, T);

            template <typename T>
            friend void BadExtern::g(T);
        };

        template <typename T>
        class friendship_GoodTemplate
        {
        };

        class friendship_GoodDeclared
        {
        public:
            void f() const;
            class Nested;
            template <typename T> void g(T);
        };

        class friendship_GoodLocal
        {
            friend void friendship_GoodDeclared::f() const;
            friend class friendship_GoodDeclared::Nested;
            template <typename T>
            friend void friendship_GoodDeclared::g(T);
        };
    }
}

#endif
