// csabbg_allocatorforward.t.cpp                                      -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csabbg_allocatorforward.t.hpp"
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabbg
    {
        namespace
        {
            class test:
                public cool::csabbg::allocatorforward_alloc_unused,
                public cool::csabbg::allocatorforward_alloc_used
            {
            public:
                test(int);                                          // IMPLICIT
                test(int, BloombergLP::bslma_Allocator*);
                test(bool, BloombergLP::bslma_Allocator*);
            private:
                BloombergLP::bslma_Allocator* allocator_;
                cool::csabbg::allocatorforward_alloc_unused    unused_;
                cool::csabbg::allocatorforward_alloc_used      used0_;
                cool::csabbg::allocatorforward_alloc_used      used1_;
                cool::csabbg::allocatorforward_alloc_used      used2_;
            };
        }
    }
}

// -----------------------------------------------------------------------------

cool::csabbg::test::test(int i):
    unused_(),
    used1_(i)
{
}

// -----------------------------------------------------------------------------

cool::csabbg::test::test(int i, BloombergLP::bslma_Allocator* alloc):
    allocatorforward_alloc_used(i),
    used0_(i),
    used1_(i, alloc)
{
}

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabbg
    {
        namespace
        {
            int dummy(BloombergLP::bslma_Allocator* a)
            {
                return a? 1: 0;
            }
        }
    }
}

cool::csabbg::test::test(bool, BloombergLP::bslma_Allocator* alloc):
    allocatorforward_alloc_used(-1, alloc),
    allocator_(alloc),
    used0_(cool::csabbg::dummy(alloc)),
    used1_(1, this->allocator_),
    used2_(2, alloc)
{
}

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabbg
    {
        namespace
        {
            template <typename T>
            struct tbase
            {
                tbase();
                explicit tbase(BloombergLP::bslma_Allocator*);
                tbase(int, BloombergLP::bslma_Allocator*);
                cool::csabbg::allocatorforward_alloc_unused unused_;
                cool::csabbg::allocatorforward_alloc_used   used0_;
            };

            template <typename T>
            tbase<T>::tbase()
            {
            }

            template <typename T>
            tbase<T>::tbase(BloombergLP::bslma_Allocator*)
            {
            }

            template <typename T>
            tbase<T>::tbase(int i, BloombergLP::bslma_Allocator* alloc):
                used0_(i, alloc)
            {
            }
        }
    }
}

void
cool::csabbg::allocatorforward_f()
{
    cool::csabbg::tbase<int> tb0;
    cool::csabbg::tbase<int> tb1(static_cast<BloombergLP::bslma_Allocator*>(0));
    cool::csabbg::tbase<int> tb2(2, static_cast<BloombergLP::bslma_Allocator*>(0));
}
