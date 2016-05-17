#include <bdeut_nullablevalue.h>
#include <bslma_allocator.h>
#include <bsls_types.h>
#include <bsl_string.h>

using namespace BloombergLP;

struct X {
    bdeut_NullableValue<bsls::Types::Uint64> u;
    bdeut_NullableValue<bsl::string> s;
    X(bslma::Allocator *a = 0) : s(a) { }
    X(const X& o, bslma::Allocator *a = 0) : u(o.u), s(o.s, a) { }
    BSLMF_NESTED_TRAIT_DECLARATION(X, bslma::UsesBslmaAllocator);
};

int main()
{
    X x;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2016 Bloomberg Finance L.P.
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
