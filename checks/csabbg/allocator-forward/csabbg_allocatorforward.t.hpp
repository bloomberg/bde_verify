// csabbg_allocatorforward.t.hpp                                      -*-C++-*-

#ifndef INCLUDED_CSABBG_ALLOCATORFORWARD
#define INCLUDED_CSABBG_ALLOCATORFORWARD

#ifndef INCLUDED_BDES_IDENT
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

// -----------------------------------------------------------------------------

namespace BloombergLP
{
    namespace bslma
    {
        class Allocator;

        template <typename TYPE> struct UsesBslmaAllocator;
    }

}

namespace bde_verify
{
    namespace csabbg
    {
        class allocatorforward_alloc_unused;
        class allocatorforward_alloc_used;
        void operator+(allocatorforward_alloc_used);
    }
}

// -----------------------------------------------------------------------------

class bde_verify::csabbg::allocatorforward_alloc_unused
{
};

// -----------------------------------------------------------------------------

class bde_verify::csabbg::allocatorforward_alloc_used
{
public:
    //allocatorforward_alloc_used();
    //allocatorforward_alloc_used(int);
                                                                    // IMPLICIT
    explicit allocatorforward_alloc_used(int = 0,
                                         BloombergLP::bslma::Allocator* = 0);
};

// -----------------------------------------------------------------------------

#endif

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
