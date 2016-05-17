// csamisc_thrownonstdexception.t.cpp                                 -*-C++-*-

#include "csamisc_thrownonstdexception.t.hpp"
#include <bdes_ident.h>

#include <exception>

namespace bde_verify
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
                throw bar();
            }
        }

        namespace
        {
            foo f;
        }
    }
}

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
