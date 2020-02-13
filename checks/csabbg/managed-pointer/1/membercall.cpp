#include <bsl_memory.h>
#include <bslma_default.h>
using namespace BloombergLP;

struct tccs {
    bslma::Allocator *a;
    tccs() : a(bslma::Default::allocator()) { }
    static tccs *s() { static tccs t; return &t; }
    bslma_Allocator *l() const { return a; }
};

struct tch {
};

struct tchs {
    bsl::shared_ptr<tch> h;
    tchs() : h(new (*tccs::s()->l()) tch(), tccs::s()->l()) { }
    tchs(int) : h(new (*(*tccs::s()).l()) tch(), (*tccs::s()).l()) { }
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
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
