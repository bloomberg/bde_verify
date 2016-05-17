// csastil_templatetypename.t.cpp                                     -*-C++-*-

#include "csastil_templatetypename.t.h"
#include <bdes_ident.h>

template <typename T>
static void bad();

template <typename TT>
static void good();

namespace bde_verify
{
    namespace csastil
    {
        namespace
        {
            template <typename T>
            static void localBad();

            template <class TT>
            static void localGood();
        }

        template <typename AA>
        struct BB
        {
            template <typename CC>
            void f();
        };
    }
}

template <typename YY>
template <typename CC>
void bde_verify::csastil::BB<YY>::f()
{
}

namespace AA {
namespace BB {
    template <typename CC>
    struct DD {
        template <typename EE>
        void f();
    };
}
}

template <typename GG>
template <typename HH>
void AA::BB::DD<GG>::f()
{
}

namespace II {
namespace JJ {
    template <typename KK>
    struct LL
    {
        template <typename MM>
        struct NN;
    };
}
}

template <typename OO>
template <typename PP>
struct II::JJ::LL<OO>::NN
{
};

template<template <class> class QQ>
void RR()
{
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
