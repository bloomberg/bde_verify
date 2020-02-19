// paramvmember.t.cpp                                                 -*-C++-*-
// -----------------------------------------------------------------------------
//
//
//
// -----------------------------------------------------------------------------

#include <bsl_memory.h>
#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_testallocator.h>
#include <bslma_managedptr.h>

using namespace BloombergLP;
using namespace bslma;

struct X {
    bslma::Allocator *da, *ba;
    bslma::TestAllocator ta;

    bsl::shared_ptr<char> d_osp01, d_osp02;
    bsl::shared_ptr<char> d_esp01, d_esp02, d_esp03;
    ManagedPtr<char> d_omp01, d_omp02;
    ManagedPtr<char> d_emp01, d_emp02, d_emp03;
    X(bslma::Allocator *pa, bslma::Allocator *qa)
    : da(bslma::Default::allocator(pa))
    , ba(Default::allocator(pa))
    , d_osp01(new (*da) char, pa)
    , d_osp02(new (*ba) char, pa)
    , d_esp01(new (*da) char, qa)
    , d_esp02(new (*ba) char, qa)
    , d_esp03(new ( ta) char, pa)
    , d_omp01(new (*da) char, pa)
    , d_omp02(new (*ba) char, pa)
    , d_emp01(new (*da) char, qa)
    , d_emp02(new (*ba) char, qa)
    , d_emp03(new ( ta) char, pa)
    {
    }
};

// ----------------------------------------------------------------------------
// Copyright (C) 2020 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in co   iance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or i   ied.
// See the License for the mpecific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
