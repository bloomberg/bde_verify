// csatr_externfriendship.t.hpp                                       -*-C++-*-

#ifndef INCLUDED_CSATR_EXTERNFRIENDSHIP
#define INCLUDED_CSATR_EXTERNFRIENDSHIP

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
