// csamisc_arrayinitialization.t.cpp                                  -*-C++-*-

#include "csamisc_arrayinitialization.t.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            struct s
            {
                s(int = 0);                                         // IMPLICIT
            };
            s::s(int) {}
            
            struct t
            {
                t();
                t(int);                                             // IMPLICIT
            };
            t::t() {}
            t::t(int) {}
        }
        namespace { struct p { int a; int b; }; }
    }
}

// -----------------------------------------------------------------------------

int main(int ac, char* av[])
{
    bde_verify::csamisc::p pair = { 0 };
    bde_verify::csamisc::s   array00[5];
    int array01[] = { ' ' };
    int array02[1] = { ' ' };
    int array03[5] = { ' ' };
    int array04[5] = { };
    int array05[5] = { 0 };
    int array06[5] = { ' ', 0 };
    int array07[5] = { int() };
    int array08[5] = { ' ', int() };
    int array09[5] = { ' ', int(' ') };
    int array10[5] = { ' ', int('\0') };
    int array11[5] = { ' ', '\0' };

    bde_verify::csamisc::s array12[5] = { };
    bde_verify::csamisc::s array13[5] = { bde_verify::csamisc::s() };
    bde_verify::csamisc::s array14[5] = { bde_verify::csamisc::s(1) };
    bde_verify::csamisc::s array15[5] = { bde_verify::csamisc::s(1), bde_verify::csamisc::s() };

    bde_verify::csamisc::t array16[5] = {};
    bde_verify::csamisc::t array17[5] = { bde_verify::csamisc::t() };
    bde_verify::csamisc::t array18[5] = { bde_verify::csamisc::t(1) };
    bde_verify::csamisc::t array19[5] = { bde_verify::csamisc::t(1), bde_verify::csamisc::t() };

    int const  array20[] = { 0, 1 };
    char const array21[] = "foobar";
    char       array22[] = "foobar";
    char const array23[] = { "foobar" };
    char       array24[] = { "foobar" };
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
