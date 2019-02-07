namespace BloombergLP { namespace bslma { class Allocator { }; } }
// BDE-VERIFY pragma: -AT02
// BDE_VERIFY pragma: -ALM01
// BDE_VERIFY pragma: -AP02
class V;
template <class> class U;

using namespace BloombergLP;

namespace {
struct MMyAllocator : public bslma::Allocator
{
    MMyAllocator(bslma::Allocator *allocator) { }
};
}

namespace BloombergLP {
namespace XX {
struct YY {
    YY(bslma::Allocator *allocator) { }
};
}
}

namespace AA {
namespace {
namespace BB {
struct YY {
    YY(bslma::Allocator *allocator) {}
};
}
}
}

namespace AA {
namespace {
namespace BB {
namespace {
namespace CC {
struct YY {
    YY(bslma::Allocator *allocator) {}
};
}
}
}
}
}

namespace {
namespace BB {
namespace {
struct YY {
    YY(bslma::Allocator *allocator) {}
};
}
}
}

struct WW {
    template <class = V, int = 7, template <class> class = U>
    struct YY {
        YY(bslma::Allocator *allocator) {}
    };
};

namespace {
template <class = V, int = 7, template <class> class = U>
struct MyAllocator : public bslma::Allocator
{
    MyAllocator(bslma::Allocator *allocator) { }
};
}

namespace BloombergLP {
namespace X {
template <class = V, int = 7, template <class> class = U>
struct Y {
    Y(bslma::Allocator *allocator) { }
};
}
}

namespace A {
namespace {
namespace B {
template <class = V, int = 7, template <class> class = U>
struct Y {
    Y(bslma::Allocator *allocator) {}
};
}
}
}

namespace A {
namespace {
namespace B {
namespace {
namespace C {
template <class = V, int = 7, template <class> class = U>
struct Y {
    Y(bslma::Allocator *allocator) {}
};
}
}
}
}
}

namespace {
namespace B {
namespace {
template <class = V, int = 7, template <class> class = U>
struct Y {
    Y(bslma::Allocator *allocator) {}
};
}
}
}

template <class T> struct Y;
template <class T> struct Y<T*> {
    Y(bslma::Allocator *allocator) {}
};
template <> struct Y<int> {
    Y(bslma::Allocator *allocator) {}
};

struct X {
  private:
    struct Y {
        Y(bslma::Allocator *allocator) {}
    };
};

template <class = V, int = 7, template <class> class = U>
struct W {
    template <class = V, int = 7, template <class> class = U>
    struct Y {
        Y(bslma::Allocator *allocator) {}
    };
};

struct M {
    template <class = V, int = 7, template <class> class = U>
    struct Y { };
};

template <class T>
struct M::Y<Y<T>, 1, Y> {
    Y(bslma::Allocator *allocator) {}
};

template <class T, int N, template <class> class X>
struct M::Y<T*, N, X> {
    Y(bslma::Allocator *allocator) {}
};

// ----------------------------------------------------------------------------
// Copyright (C) 2017 Bloomberg Finance L.P.
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
