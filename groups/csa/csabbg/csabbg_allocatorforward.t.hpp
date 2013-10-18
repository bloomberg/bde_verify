// csabbg_allocatorforward.t.hpp                                      -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABBG_ALLOCATORFORWARD)
#define INCLUDED_CSABBG_ALLOCATORFORWARD 1
#ifndef INCLUDED_BDES_IDENT
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

// -----------------------------------------------------------------------------

namespace BloombergLP
{
    namespace bslma { class Allocator; }
}

namespace cool
{
    namespace csabbg
    {
        class allocatorforward_alloc_unused;
        class allocatorforward_alloc_used;
        void operator+(allocatorforward_alloc_used);
    }
}

// -----------------------------------------------------------------------------

class cool::csabbg::allocatorforward_alloc_unused
{
};

// -----------------------------------------------------------------------------

class cool::csabbg::allocatorforward_alloc_used
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
