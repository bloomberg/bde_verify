// csamisc_memberdefinitioninclassdefinition.v.cpp                    -*-C++-*-

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
