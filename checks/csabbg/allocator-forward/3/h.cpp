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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
