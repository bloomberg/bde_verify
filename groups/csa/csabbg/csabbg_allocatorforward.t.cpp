// csabbg_allocatorforward.t.cpp                                      -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csabbg_allocatorforward.t.hpp"
#include <bdes_ident.h>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabbg
    {
        namespace
        {
            class test:
                public bde_verify::csabbg::allocatorforward_alloc_unused,
                public bde_verify::csabbg::allocatorforward_alloc_used
            {
            public:
                test(int);                                          // IMPLICIT
                test(int, BloombergLP::bslma::Allocator*);
                test(bool, BloombergLP::bslma::Allocator*);
            private:
                BloombergLP::bslma::Allocator* allocator_;
                bde_verify::csabbg::allocatorforward_alloc_unused    unused_;
                bde_verify::csabbg::allocatorforward_alloc_used      used0_;
                bde_verify::csabbg::allocatorforward_alloc_used      used1_;
                bde_verify::csabbg::allocatorforward_alloc_used      used2_;
            };
        }
    }
}

// -----------------------------------------------------------------------------

bde_verify::csabbg::test::test(int i):
    unused_(),
    used1_(i)
{
}

// -----------------------------------------------------------------------------

bde_verify::csabbg::test::test(int i, BloombergLP::bslma::Allocator* alloc):
    allocatorforward_alloc_used(i),
    used0_(i),
    used1_(i, alloc)
{
}

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabbg
    {
        namespace
        {
            int dummy(BloombergLP::bslma::Allocator* a)
            {
                return a? 1: 0;
            }
        }
    }
}

bde_verify::csabbg::test::test(bool, BloombergLP::bslma::Allocator* alloc):
    allocatorforward_alloc_used(-1, alloc),
    allocator_(alloc),
    used0_(bde_verify::csabbg::dummy(alloc)),
    used1_(1, allocator_),
    used2_(2, alloc)
{
}

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabbg
    {
        namespace
        {
            template <class T>
            struct tbase
            {
                tbase();
                explicit tbase(BloombergLP::bslma::Allocator*);
                tbase(int, BloombergLP::bslma::Allocator*);
                bde_verify::csabbg::allocatorforward_alloc_unused unused_;
                bde_verify::csabbg::allocatorforward_alloc_used   used0_;
            };

            template <class T>
            tbase<T>::tbase()
            {
            }

            template <class T>
            tbase<T>::tbase(BloombergLP::bslma::Allocator*)
            {
            }

            template <class T>
            tbase<T>::tbase(int i, BloombergLP::bslma::Allocator* alloc):
                used0_(i, alloc)
            {
            }
        }
    }
}

void
bde_verify::csabbg::operator+(allocatorforward_alloc_used)
{
    bde_verify::csabbg::tbase<int> tb0;
    bde_verify::csabbg::tbase<int> tb1(static_cast<BloombergLP::bslma::Allocator*>(0));
    bde_verify::csabbg::tbase<int> tb2(2, static_cast<BloombergLP::bslma::Allocator*>(0));
}

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabbg
    {
        namespace
        {
            template <class T>
                struct M { M(BloombergLP::bslma::Allocator*) {} };
            template <class T, class A = M<T> >
                struct S { S(const T*) {} S(const T*, const A&) {} };
            template class M<char>;  // TBD should not be needed
            struct C {
                S<char> s;
                C(BloombergLP::bslma::Allocator*) : s("") { }
            };
        }
    }
}

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabbg
    {
        struct not_alloc {
            not_alloc(BloombergLP::bslma::Allocator*) { }
        };
        struct object {
            not_alloc member;
            object(not_alloc na, BloombergLP::bslma::Allocator* = 0)
            : member(na) { }
        };
    }
}

namespace BloombergLP
{
    namespace bslma
    {
        template <>
        struct UsesBslmaAllocator<bde_verify::csabbg::object> : bsl::true_type
        {
        };

    }
}

namespace bde_verify
{
    namespace csabbg
    {
        void test(BloombergLP::bslma::Allocator *a)
        {
            object o(a);
        }
        object ra(bool b, const object &o)
        {
            if (b) {
                return o;
            } else {
                return object(o);
            }
        }
    }
}
