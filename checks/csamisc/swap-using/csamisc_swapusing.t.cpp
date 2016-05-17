// csamisc_swapusing.t.cpp                                            -*-C++-*-
#include <utility>

struct V { };
void swap(V&, V&);
struct W { };
void swap(W&, W&);

void f()
{
    int i1{};
    int i2{};
    V v1{};
    V v2{};
    W w1{};
    W w2{};

    std::swap(i1, i2);
    ::std::swap(i1, i2);
    std::swap(v1, v2);
    ::std::swap(v1, v2);
    swap(v1, v2);
    std::swap(w1, w2);
    ::std::swap(w1, w2);
    swap(w1, w2);

    using std::swap;

    std::swap(i1, i2);
    ::std::swap(i1, i2);
    swap(i1, i2);
    std::swap(v1, v2);
    ::std::swap(v1, v2);
    swap(v1, v2);
    std::swap(w1, w2);
    ::std::swap(w1, w2);
    swap(w1, w2);
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
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
