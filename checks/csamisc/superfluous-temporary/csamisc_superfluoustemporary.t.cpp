// csamisc_superfluoustemporary.t.cpp                                 -*-C++-*-

#include "csamisc_superfluoustemporary.t.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            struct bar
            {
            };

            struct foo
            {
                foo();
                foo(foo const&);
                foo(bar const&);                                    // IMPLICIT
            };
            foo::foo() {}
            foo::foo(foo const&) {}
            foo::foo(bar const&) {}
        }
    }
}

int main(int ac, char* av[])
{
    bde_verify::csamisc::foo f0 = bde_verify::csamisc::foo();
    bde_verify::csamisc::foo b0 = bde_verify::csamisc::bar();
    //foo f1;
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
