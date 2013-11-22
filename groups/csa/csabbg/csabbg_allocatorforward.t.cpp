// csabbg_allocatorforward.t.cpp                                      -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csabbg_allocatorforward.t.hpp"
#include <bdes_ident.h>

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
                test(int, BloombergLP::bslma::Allocator*);
                test(bool, BloombergLP::bslma::Allocator*);
            private:
                BloombergLP::bslma::Allocator* allocator_;
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

cool::csabbg::test::test(int i, BloombergLP::bslma::Allocator* alloc):
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
            int dummy(BloombergLP::bslma::Allocator* a)
            {
                return a? 1: 0;
            }
        }
    }
}

cool::csabbg::test::test(bool, BloombergLP::bslma::Allocator* alloc):
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
            template <class T>
            struct tbase
            {
                tbase();
                explicit tbase(BloombergLP::bslma::Allocator*);
                tbase(int, BloombergLP::bslma::Allocator*);
                cool::csabbg::allocatorforward_alloc_unused unused_;
                cool::csabbg::allocatorforward_alloc_used   used0_;
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
cool::csabbg::operator+(allocatorforward_alloc_used)
{
    cool::csabbg::tbase<int> tb0;
    cool::csabbg::tbase<int> tb1(static_cast<BloombergLP::bslma::Allocator*>(0));
    cool::csabbg::tbase<int> tb2(2, static_cast<BloombergLP::bslma::Allocator*>(0));
}

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabbg
    {
        namespace
        {
            template <class T>
                struct M { M(BloombergLP::bslma::Allocator*); };
            template <class T, class A = M<T> >
                struct S { S(const T*); S(const T*, const A&); };
            template class M<char>;  // TBD should not be needed
            struct C {
                S<char> s;
                C(BloombergLP::bslma::Allocator*) : s("") { }
            };
        }
    }
}

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabbg
    {
        namespace
        {
            struct not_alloc {
                not_alloc(BloombergLP::bslma::Allocator*) { }
            };
            struct object {
                not_alloc member;
                object(not_alloc na, BloombergLP::bslma::Allocator* = 0)
                : member(na) { }
            };
            void test(BloombergLP::bslma::Allocator *a)
            {
                object o(a);
            }
        }
    }
}
