namespace BloombergLP {
namespace bslma { class Allocator; }
}
using namespace BloombergLP;
struct BaseWithoutAllocator {
};
template <int N>
struct BaseWithAllocator {
    BaseWithAllocator(bslma::Allocator *);
};
struct FieldWithoutAllocator {
};
struct FieldWithAllocator {
    FieldWithAllocator(bslma::Allocator *);
};
struct C1 : BaseWithoutAllocator, BaseWithAllocator<1>, BaseWithAllocator<2> {
    bslma::Allocator *d_allocator_p;
    C1(bslma::Allocator *);
};
struct C2 {
    FieldWithoutAllocator  d_f1;
    FieldWithAllocator     d_f2;
    FieldWithAllocator     d_f3;
    bslma::Allocator      *d_allocator_p;
    C2(bslma::Allocator *);
};
struct C3 : BaseWithoutAllocator, BaseWithAllocator<1>, BaseWithAllocator<2> {
    FieldWithoutAllocator  d_f1;
    FieldWithAllocator     d_f2;
    FieldWithAllocator     d_f3;
    bslma::Allocator      *d_allocator_p;
    C3(bslma::Allocator *);
};

// ----------------------------------------------------------------------------
// Copyright (C) 2018 Bloomberg Finance L.P.
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
