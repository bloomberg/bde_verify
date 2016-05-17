// csastil_uppernames.t.cpp                                           -*-C++-*-

#include <stdio.h>

namespace bde_verify {
namespace csastil {
namespace {

int AA;

void BB()
{
    int BB_AA;
    struct BB_CC
    {
    };
}

struct CC
{
    int CC_AA;
    template <class CC_DD> struct CC_EE { };
};

}

template <class DD> struct EE
{
    int EE_AA;
    struct EE_CC  { };
    template <class EE_DD> struct EE_EE { };
    template <class EE_FF> struct EE_GG;
};

}
}

template <class EE_HH>
template <class EE_II>
struct bde_verify::csastil::EE<EE_HH>::EE_GG
{
    EE_HH JJ;
    EE_II KK;
};

int LL_CSASTIL_UPPERNAMES;

#define BIG BIG_NAME

int BIG;

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
