// array.hpp                                                          -*-C++-*-

#ifndef INCLUDED_UTILS_ARRAY_HPP
#define INCLUDED_UTILS_ARRAY_HPP

#include <stddef.h>

// -----------------------------------------------------------------------------

namespace utils
{
template <typename T, size_t Size>
T* begin(T (&array)[Size])
{
    return array;
}

template <typename T, size_t Size>
T* end(T (&array)[Size])
{
    return array + Size;
}
}

// -----------------------------------------------------------------------------

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
