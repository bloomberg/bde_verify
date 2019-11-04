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
