// csamisc_constantreturn.t.cpp                                       -*-C++-*-

#include "csamisc_constantreturn.t.hpp"
#include <bdes_ident.h>

int bde_verify::csamisc::constantreturn::f()
{
    return 0;
}

int bde_verify::csamisc::constantreturn::g()
{
    return 5 + 8;
}

int bde_verify::csamisc::constantreturn::h()
{
    {
        {
            return constantreturn::x();
        }
    }
}

void bde_verify::csamisc::constantreturn::v()
{
    return;
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
