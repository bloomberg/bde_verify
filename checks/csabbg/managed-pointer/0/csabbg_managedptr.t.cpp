// csabbg_managedptr.t.cpp                                            -*-C++-*-
// -----------------------------------------------------------------------------
//
//
//
// -----------------------------------------------------------------------------

#include <bsl_memory.h>
#include <bslma_allocator.h>
#include <bslma_default.h>
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

    bsl::shared_ptr<char> sp01(new char);
    bsl::shared_ptr<char> sp02(new (*pa) char);
    bsl::shared_ptr<char> sp03(new (*pa) char, pa);
    bsl::shared_ptr<char> sp04(new (*da) char);
    bsl::shared_ptr<char> sp05(new (*da) char, da);
    bsl::shared_ptr<char> sp06(new (*ba) char);
    bsl::shared_ptr<char> sp07(new (*ba) char, ba);
    bsl::shared_ptr<char> sp08(new (ta) char);
    bsl::shared_ptr<char> sp09(new (ta) char, &ta);
    bsl::shared_ptr<char> sp10(new (*bslma::Default::allocator()) char);
    bsl::shared_ptr<char> sp11(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator());
    bsl::shared_ptr<char> sp12(new (*Default::allocator()) char);
    bsl::shared_ptr<char> sp13(new (*Default::allocator()) char,
                          Default::allocator());
    bsl::shared_ptr<char> sp14(new (*aa) char);
    bsl::shared_ptr<char> sp15(new (*aa) char, aa);
    bsl::shared_ptr<char> sp16(new (*ba) char);
    bsl::shared_ptr<char> sp17(new (*ba) char, ba);
    bsl::shared_ptr<char> sp18(new (*za) char);
    bsl::shared_ptr<char> sp19(new (*za) char, za);

    bsl::shared_ptr<char>(new char);
    bsl::shared_ptr<char>(new (*pa) char);
    bsl::shared_ptr<char>(new (*pa) char, pa);
    bsl::shared_ptr<char>(new (*da) char);
    bsl::shared_ptr<char>(new (*da) char, da);
    bsl::shared_ptr<char>(new (*ba) char);
    bsl::shared_ptr<char>(new (*ba) char, ba);
    bsl::shared_ptr<char>(new (ta) char);
    bsl::shared_ptr<char>(new (ta) char, &ta);
    bsl::shared_ptr<char>(new (*bslma::Default::allocator()) char);
    bsl::shared_ptr<char>(new (*bslma::Default::allocator()) char,
                     bslma::Default::allocator());
    bsl::shared_ptr<char>(new (*Default::allocator()) char);
    bsl::shared_ptr<char>(new (*Default::allocator()) char, Default::allocator());
    bsl::shared_ptr<char>(new (*aa) char);
    bsl::shared_ptr<char>(new (*aa) char, aa);
    bsl::shared_ptr<char>(new (*ba) char);
    bsl::shared_ptr<char>(new (*ba) char, ba);
    bsl::shared_ptr<char>(new (*za) char);
    bsl::shared_ptr<char>(new (*za) char, za);

    bsl::shared_ptr<char> ep01(new char, da);
    bsl::shared_ptr<char> ep02(new (*ba) char, da);
    bsl::shared_ptr<char> ep03(new (ta) char, ba);
    bsl::shared_ptr<char> ep04(new (*da) char, &ta);
    bsl::shared_ptr<char> ep05(new (*bslma::Default::allocator()) char, da);
    bsl::shared_ptr<char> ep06(new (*Default::allocator()) char, da);
    bsl::shared_ptr<char> ep07(new (*da) char, Default::allocator());
    bsl::shared_ptr<char> ep08(new (*da) char, bslma::Default::allocator());
}

struct X {
    bslma::Allocator *da, *ba;
    bslma::TestAllocator ta;

    bsl::shared_ptr<char> d_sp01, d_sp02, d_sp03, d_sp04, d_sp05, d_sp06, d_sp07;
    bsl::shared_ptr<char> d_sp08, d_sp09, d_sp10, d_sp11, d_sp12, d_sp13, d_sp14;
    bsl::shared_ptr<char> d_sp15, d_sp16, d_sp17, d_sp18, d_sp19;
    bsl::shared_ptr<char> d_ep01, d_ep02, d_ep03, d_ep04, d_ep05, d_ep06;
    bsl::shared_ptr<char> d_ep07, d_ep08;
    X(bslma::Allocator *pa)
    : da(bslma::Default::allocator())
    , ba(Default::allocator())
    , d_sp01(new char)
    , d_sp02(new (*pa) char)
    , d_sp03(new (*pa) char, pa)
    , d_sp04(new (*da) char)
    , d_sp05(new (*da) char, da)
    , d_sp06(new (*ba) char)
    , d_sp07(new (*ba) char, ba)
    , d_sp08(new (ta) char)
    , d_sp09(new (ta) char, &ta)
    , d_sp10(new (*bslma::Default::allocator()) char)
    , d_sp11(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator())
    , d_sp12(new (*Default::allocator()) char)
    , d_sp13(new (*Default::allocator()) char, Default::allocator())
    , d_sp16(new (*ba) char)
    , d_sp17(new (*ba) char, ba)
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

void g(bsl::shared_ptr<char> &sp, bslma::Allocator *pa)
{
    bslma::Allocator     *da = bslma::Default::allocator();
    bslma::Allocator     *ba = Default::allocator();
    bslma::Allocator     *aa;
    bslma::Allocator     *za;
    bslma::TestAllocator  ta;

    aa = bslma::Default::allocator();
    za = Default::allocator();

    sp.reset(new char);
    sp.reset(new (*pa) char);
    sp.reset(new (*pa) char, pa);
    sp.reset(new (*da) char);
    sp.reset(new (*da) char, da);
    sp.reset(new (*ba) char);
    sp.reset(new (*ba) char, ba);
    sp.reset(new (ta) char);
    sp.reset(new (ta) char, &ta);
    sp.reset(new (*bslma::Default::allocator()) char);
    sp.reset(new (*bslma::Default::allocator()) char,
                          bslma::Default::allocator());
    sp.reset(new (*Default::allocator()) char);
    sp.reset(new (*Default::allocator()) char,
                          Default::allocator());
    sp.reset(new (*aa) char);
    sp.reset(new (*aa) char, aa);
    sp.reset(new (*ba) char);
    sp.reset(new (*ba) char, ba);
    sp.reset(new (*za) char);
    sp.reset(new (*za) char, za);

    sp.reset(new char);
    sp.reset(new (*pa) char);
    sp.reset(new (*pa) char, pa);
    sp.reset(new (*da) char);
    sp.reset(new (*da) char, da);
    sp.reset(new (*ba) char);
    sp.reset(new (*ba) char, ba);
    sp.reset(new (ta) char);
    sp.reset(new (ta) char, &ta);
    sp.reset(new (*bslma::Default::allocator()) char);
    sp.reset(new (*bslma::Default::allocator()) char,
                     bslma::Default::allocator());
    sp.reset(new (*Default::allocator()) char);
    sp.reset(new (*Default::allocator()) char, Default::allocator());
    sp.reset(new (*aa) char);
    sp.reset(new (*aa) char, aa);
    sp.reset(new (*ba) char);
    sp.reset(new (*ba) char, ba);
    sp.reset(new (*za) char);
    sp.reset(new (*za) char, za);

    sp.reset(new char, da);
    sp.reset(new (*ba) char, da);
    sp.reset(new (ta) char, ba);
    sp.reset(new (*da) char, &ta);
    sp.reset(new (*bslma::Default::allocator()) char, da);
    sp.reset(new (*Default::allocator()) char, da);
    sp.reset(new (*da) char, Default::allocator());
    sp.reset(new (*da) char, bslma::Default::allocator());
}

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
