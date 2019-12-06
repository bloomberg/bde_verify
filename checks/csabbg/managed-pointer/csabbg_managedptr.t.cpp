// csabbg_managedptr.t.cpp                                            -*-C++-*-
// -----------------------------------------------------------------------------
//
//
//
// -----------------------------------------------------------------------------

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_managedptr.h>
#include <bslma_testallocator.h>

using namespace BloombergLP;
using namespace bslma;

void f(bslma::Allocator *pa)
{
    bslma::Allocator     *da = bslma::Default::allocator();
    bslma::Allocator     *ba = Default::allocator();
    bslma::Allocator     *aa;
    bslma::Allocator     *za;
    bslma::TestAllocator  ta;

    aa = bslma::Default::allocator();
    za = Default::allocator();

    ManagedPtr<char> mp01(new char);
    ManagedPtr<char> mp02(new (*pa) char);
    ManagedPtr<char> mp03(new (*pa) char, pa);
    ManagedPtr<char> mp04(new (*da) char);
    ManagedPtr<char> mp05(new (*da) char, da);
    ManagedPtr<char> mp06(new (*ba) char);
    ManagedPtr<char> mp07(new (*ba) char, ba);
    ManagedPtr<char> mp08(new (ta) char);
    ManagedPtr<char> mp09(new (ta) char, &ta);
    ManagedPtr<char> mp10(new (*bslma::Default::allocator()) char);
    ManagedPtr<char> mp11(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator());
    ManagedPtr<char> mp12(new (*Default::allocator()) char);
    ManagedPtr<char> mp13(new (*Default::allocator()) char,
                          Default::allocator());
    ManagedPtr<char> mp14(new (*aa) char);
    ManagedPtr<char> mp15(new (*aa) char, aa);
    ManagedPtr<char> mp16(new (*ba) char);
    ManagedPtr<char> mp17(new (*ba) char, ba);
    ManagedPtr<char> mp18(new (*za) char);
    ManagedPtr<char> mp19(new (*za) char, za);

    ManagedPtr<char>(new char);
    ManagedPtr<char>(new (*pa) char);
    ManagedPtr<char>(new (*pa) char, pa);
    ManagedPtr<char>(new (*da) char);
    ManagedPtr<char>(new (*da) char, da);
    ManagedPtr<char>(new (*ba) char);
    ManagedPtr<char>(new (*ba) char, ba);
    ManagedPtr<char>(new (ta) char);
    ManagedPtr<char>(new (ta) char, &ta);
    ManagedPtr<char>(new (*bslma::Default::allocator()) char);
    ManagedPtr<char>(new (*bslma::Default::allocator()) char,
                     bslma::Default::allocator());
    ManagedPtr<char>(new (*Default::allocator()) char);
    ManagedPtr<char>(new (*Default::allocator()) char, Default::allocator());
    ManagedPtr<char>(new (*aa) char);
    ManagedPtr<char>(new (*aa) char, aa);
    ManagedPtr<char>(new (*ba) char);
    ManagedPtr<char>(new (*ba) char, ba);
    ManagedPtr<char>(new (*za) char);
    ManagedPtr<char>(new (*za) char, za);

    ManagedPtr<char> ep01(new char, da);
    ManagedPtr<char> ep02(new (*ba) char, da);
    ManagedPtr<char> ep03(new (ta) char, ba);
    ManagedPtr<char> ep04(new (*da) char, &ta);
    ManagedPtr<char> ep05(new (*bslma::Default::allocator()) char, da);
    ManagedPtr<char> ep06(new (*Default::allocator()) char, da);
    ManagedPtr<char> ep07(new (*da) char, Default::allocator());
    ManagedPtr<char> ep08(new (*da) char, bslma::Default::allocator());
}

struct X {
    bslma::Allocator *da, *ba;
    bslma::TestAllocator ta;

    ManagedPtr<char> d_mp01, d_mp02, d_mp03, d_mp04, d_mp05, d_mp06, d_mp07;
    ManagedPtr<char> d_mp08, d_mp09, d_mp10, d_mp11, d_mp12, d_mp13, d_mp14;
    ManagedPtr<char> d_mp15, d_mp16, d_mp17, d_mp18, d_mp19;
    ManagedPtr<char> d_ep01, d_ep02, d_ep03, d_ep04, d_ep05, d_ep06;
    ManagedPtr<char> d_ep07, d_ep08;
    X(bslma::Allocator *pa)
    : da(bslma::Default::allocator())
    , ba(Default::allocator())
    , d_mp01(new char)
    , d_mp02(new (*pa) char)
    , d_mp03(new (*pa) char, pa)
    , d_mp04(new (*da) char)
    , d_mp05(new (*da) char, da)
    , d_mp06(new (*ba) char)
    , d_mp07(new (*ba) char, ba)
    , d_mp08(new (ta) char)
    , d_mp09(new (ta) char, &ta)
    , d_mp10(new (*bslma::Default::allocator()) char)
    , d_mp11(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator())
    , d_mp12(new (*Default::allocator()) char)
    , d_mp13(new (*Default::allocator()) char, Default::allocator())
    , d_mp16(new (*ba) char)
    , d_mp17(new (*ba) char, ba)
    , d_ep01(new char, da)
    , d_ep02(new (*ba) char, da)
    , d_ep03(new (ta) char, ba)
    , d_ep04(new (*da) char, &ta)
    , d_ep05(new (*bslma::Default::allocator()) char, da)
    , d_ep06(new (*Default::allocator()) char, da)
    , d_ep07(new (*da) char, Default::allocator())
    , d_ep08(new (*da) char, bslma::Default::allocator())
    {
    }
};

void g(ManagedPtr<char> &mp, bslma::Allocator *pa)
{
    bslma::Allocator     *da = bslma::Default::allocator();
    bslma::Allocator     *ba = Default::allocator();
    bslma::Allocator     *aa;
    bslma::Allocator     *za;
    bslma::TestAllocator  ta;

    aa = bslma::Default::allocator();
    za = Default::allocator();

    mp.load(new char);
    mp.load(new (*pa) char);
    mp.load(new (*pa) char, pa);
    mp.load(new (*da) char);
    mp.load(new (*da) char, da);
    mp.load(new (*ba) char);
    mp.load(new (*ba) char, ba);
    mp.load(new (ta) char);
    mp.load(new (ta) char, &ta);
    mp.load(new (*bslma::Default::allocator()) char);
    mp.load(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator());
    mp.load(new (*Default::allocator()) char);
    mp.load(new (*Default::allocator()) char,
                          Default::allocator());
    mp.load(new (*aa) char);
    mp.load(new (*aa) char, aa);
    mp.load(new (*ba) char);
    mp.load(new (*ba) char, ba);
    mp.load(new (*za) char);
    mp.load(new (*za) char, za);

    mp.load(new char);
    mp.load(new (*pa) char);
    mp.load(new (*pa) char, pa);
    mp.load(new (*da) char);
    mp.load(new (*da) char, da);
    mp.load(new (*ba) char);
    mp.load(new (*ba) char, ba);
    mp.load(new (ta) char);
    mp.load(new (ta) char, &ta);
    mp.load(new (*bslma::Default::allocator()) char);
    mp.load(new (*bslma::Default::allocator()) char,
                     bslma::Default::allocator());
    mp.load(new (*Default::allocator()) char);
    mp.load(new (*Default::allocator()) char, Default::allocator());
    mp.load(new (*aa) char);
    mp.load(new (*aa) char, aa);
    mp.load(new (*ba) char);
    mp.load(new (*ba) char, ba);
    mp.load(new (*za) char);
    mp.load(new (*za) char, za);

    mp.load(new char, da);
    mp.load(new (*ba) char, da);
    mp.load(new (ta) char, ba);
    mp.load(new (*da) char, &ta);
    mp.load(new (*bslma::Default::allocator()) char, da);
    mp.load(new (*Default::allocator()) char, da);
    mp.load(new (*da) char, Default::allocator());
    mp.load(new (*da) char, bslma::Default::allocator());
}

// {DRQS 152850882}
struct Y {
    int n;
    bslma::Allocator *a;
    bslma::ManagedPtr<Y> p;
    void foo() { p.load(new(*a) Y(n, a), a); }
    Y(int n, bslma::Allocator *a = 0) : n(n), a(a), p() { foo(); }
};

// ----------------------------------------------------------------------------
// Copyright (C) 2019 Bloomberg Finance L.P.
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
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
