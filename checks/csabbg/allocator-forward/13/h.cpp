// Test fix for trailing default arguments (e.g., enable_if)

#include <bslma_allocator.h>
#include <bslma_usesbslmaallocator.h>

using namespace BloombergLP;

struct Item {
    Item();
    explicit Item(bslma::Allocator *, void * = 0);
    Item(const Item&, bslma::Allocator * = 0, void * = 0);
    Item& operator=(const Item&);
    bslma::Allocator * allocator() const;
    BSLMF_NESTED_TRAIT_DECLARATION(Item, bslma::UsesBslmaAllocator);
};

struct Holder {
    Item d_item;
    Holder();
    explicit Holder(bslma::Allocator *);
    Holder(const Holder&, bslma::Allocator * = 0);
    Holder& operator=(const Holder&);
    Holder(const Item& item, bslma::Allocator* a = 0) : d_item(item, a) { }
    bslma::Allocator * allocator() const;
    BSLMF_NESTED_TRAIT_DECLARATION(Holder, bslma::UsesBslmaAllocator);
};
    

// ----------------------------------------------------------------------------
// Copyright (C) 2020 Bloomberg Finance L.P.
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
