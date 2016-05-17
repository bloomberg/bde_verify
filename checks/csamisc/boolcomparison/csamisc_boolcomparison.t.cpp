// csamisc_boolcomparison.t.cpp                                       -*-C++-*-

namespace bde_verify
{
    namespace csamisc
    {
        bool boolcomparison();
    }
}

int main()
{
    typedef bool Boolean;
    bool       b0 = bde_verify::csamisc::boolcomparison();
    Boolean    b1 = bde_verify::csamisc::boolcomparison();
    bool const b2 = bde_verify::csamisc::boolcomparison();
    int  i(0);

    if (i     == 0) {}
    if (i     == true) {}
    if (i     == false) {}
    if (b0    == 0) {}
    if (b0    == true) {}
    if (b0    == false) {}
    if (b1    == 0) {}
    if (b1    == true) {}
    if (b1    == false) {}
    if (b2    == 0) {}
    if (b2    == true) {}
    if (b2    == false) {}
    if (0     == i) {}
    if (true  == i) {}
    if (false == i) {}
    if (0     == b0) {}
    if (true  == b0) {}
    if (false == b0) {}
    if (0     == b1) {}
    if (true  == b1) {}
    if (false == b1) {}
    if (0     == b2) {}
    if (true  == b2) {}
    if (false == b2) {}

    if (true  != b0) {}
    if (true  <= b0) {}
    if (true  <  b0) {}
    if (true  >= b0) {}
    if (true  >  b0) {}
    if (true  |  b0) {}
    if (true  +  b0) {}
    if (true  || b0) {}
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
