#include <bsl_vector.h>
#include <bslma_allocator.h>
#include <bslmf_nestedtraitdeclaration.h>
#include <bslma_usesbslmaallocator.h>

using namespace BloombergLP;

namespace {
    class Functor {
        bsl::vector<int> d_fields;
    public:
        BSLMF_NESTED_TRAIT_DECLARATION(Functor, bslma::UsesBslmaAllocator);

        Functor(const bsl::vector<int>&  fields,
                bslma::Allocator        *allocator)
            : d_fields(fields) {  // <--- allocator not forwarded
        }
        Functor(const Functor& other, bslma::Allocator *allocator = 0)
            : d_fields(other.d_fields) {  // <--- allocator not forwarded
        }
        Functor(int a, bslma::Allocator *allocator = 0)
            : d_fields(a, allocator) {
        }
    };
}

void f(bslma::Allocator* alloc) {
    bsl::vector<int> fields;
    Functor fun(fields, alloc);
}

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
