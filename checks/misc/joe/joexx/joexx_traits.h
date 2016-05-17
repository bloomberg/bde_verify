// joexx_traits.h                                                     -*-C++-*-
#ifndef INCLUDED_JOEXX_TRAITS
#define INCLUDED_JOEXX_TRAITS

#ifndef INCLUDED_BSLMF_ISTRIVIALLYCOPYABLE
#include <bslmf_istriviallycopyable.h>
#endif

#ifndef INCLUDED_BSLMF_NESTEDTRAITDECLARATION
#include <bslmf_nestedtraitdeclaration.h>
#endif

namespace BloombergLP {
namespace joexx {
class Traits { int a[7]; };
}  // close package namespace
}  // close enterprise namespace

namespace bsl {
template <>
struct is_trivially_copyable<BloombergLP::joexx::Traits> : true_type
{
};
}  // close namespace bsl

namespace BloombergLP {
namespace joexx {
class Traits_Bb {
    BSLMF_NESTED_TRAIT_DECLARATION(Traits_Bb, bsl::is_trivially_copyable)
};
class Traits_Cc {
    BSLMF_NESTED_TRAIT_DECLARATION(Traits_Bb, bsl::is_trivially_copyable)
};
}  // close package namespace
}  // close enterprise namespace

#endif

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
