// csamisc_swapusing.t.cpp                                            -*-C++-*-
#include <bsl_iostream.h>
#include <bsl_ostream.h>

void f()
{
    bsl::cout << bsl::endl << bsl::endl;
    using namespace bsl;
    cout << endl << endl;
}

void g()
{
    std::cout << std::endl << std::endl;
    using namespace std;
    cout << endl << endl;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
