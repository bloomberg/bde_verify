// a_pair.cpp                                                         -*-C++-*-
#define _FLOATINGPOINT_H // Avoid SunOs floatingpoint.h atof conflict.
#include <bsl_utility.h>
#include <bsl_map.h>

int main()
{
    bsl::map<int, int> m;
    m.insert(bsl::make_pair(1, 2));
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
