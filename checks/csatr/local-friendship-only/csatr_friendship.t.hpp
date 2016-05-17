// csatr_friendship.t.hpp                                             -*-C++-*-

#ifndef INCLUDED_CSATR_FRIENDSHIP
#define INCLUDED_CSATR_FRIENDSHIP

#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

#ifndef INCLUDED_CSATR_EXTERNFRIENDSHIP
#include "csatr_externfriendship.t.hpp"
#endif

namespace bde_verify
{
    namespace csamisc
    {
        class BadGroup;
    }

    namespace csatr
    {
        class BadPackage;
        template <class> class friendship_GoodTemplate;
        class friendship_GoodDeclared;
        void operator+ (friendship_GoodDeclared const&);
        template <class T>
        void operator+ (friendship_GoodDeclared const&, T);

        class friendship_Component
        {
        public:
            friend class bde_verify::csamisc::BadGroup;
            friend class bde_verify::csatr::BadPackage;
            friend class bde_verify::csatr::BadExtern;
            friend void bde_verify::csatr::BadExtern::f() const;
            friend class bde_verify::csatr::BadExtern::Nested;
            friend class bde_verify::csatr::friendship_GoodDeclared;
            friend class friendship_GoodLocal;
            template <class T> friend class BadPackageTemplate;
            template <class T> friend class GoodTemplate;
            friend void bde_verify::csatr::operator+ (BadExtern const&);
            template <class T>
            friend void bde_verify::csatr::operator+ (BadExtern const&, T);

            friend void bde_verify::csatr::operator+ (friendship_GoodDeclared const&);
            template <class T>
            friend void bde_verify::csatr::operator+ (friendship_GoodDeclared const&, T);

            template <class T>
            friend void BadExtern::g(T);
        };

        template <class T>
        class friendship_GoodTemplate
        {
        };

        class friendship_GoodDeclared
        {
        public:
            void f() const;
            class Nested;
            template <class T> void g(T);
        };

        class friendship_GoodLocal
        {
            friend void friendship_GoodDeclared::f() const;
            friend class friendship_GoodDeclared::Nested;
            template <class T>
            friend void friendship_GoodDeclared::g(T);
        };

        class FriendshipLocal
        {
        private:
            struct PImpl;
            friend struct PImpl;
        };

        template <class T>
        struct friendship_GoodTemplateDeclared;

        template <class T>
        struct FriendlyToDeclared
        {
            friend struct friendship_GoodTemplateDeclared<T>;
        };

        template <class T>
        struct friendship_GoodTemplateDeclared
        {
        };
    }
}

#endif

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
