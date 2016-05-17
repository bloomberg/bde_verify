// csatr_componentheaderinclude.v.cpp                                 -*-C++-*-

#include <bdes_ident.h>
BDES_IDENT_RCSID(csatr_componentheaderinclude_v_cpp, "$Id$ $CSID$")

namespace bde_verify
{
    namespace csatr
    {
        static int local(0);
    }
}

#include "csatr_componentheaderinclude.v.hpp"

int bde_verify::csatr::componentheaderinclude::something(17);

int main()
{
    return bde_verify::csatr::local;
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
