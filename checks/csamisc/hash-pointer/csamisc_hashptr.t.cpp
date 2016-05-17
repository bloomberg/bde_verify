// csamisc_hashptr.t.cpp                                              -*-C++-*-
#if __cplusplus >= 201103

#include <functional>

namespace bde_verify {
namespace {

void f()
{
    std::hash<                        char    *>()( "abc");
    std::hash<const                   char    *>()( "abc");
    std::hash<                        wchar_t *>()(L"abc");
    std::hash<const                   wchar_t *>()(L"abc");
    std::hash<const                   void    *>()( "abc");
    double d;
    std::hash<double *>()(&d);
}

}
}

#endif  // __cplusplus >= 201103

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
