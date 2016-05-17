// csamisc_selfinitialization.t.cpp                                   -*-C++-*-

#include "csamisc_selfinitialization.t.hpp"
#include <bdes_ident.h>

int main()
{
    int f1(f1);
    int f2(f2 + 1);
    int f4(f2);

    int f3((f3=0,f3 + 1));
    int* p(new int[sizeof(p)]);
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
