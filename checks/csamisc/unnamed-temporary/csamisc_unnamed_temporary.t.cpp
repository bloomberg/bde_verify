// csamisc_unnamed_temporary.t.cpp                                    -*-C++-*-

#include <string>

namespace bde_verify {
namespace {

std::string f()
{
    int i1(5);
    int(5);
    (void)int(5);
    int i2(int(5));

    std::string s1("5");
    std::string("5");
    (void)std::string("5");
    std::string s2(std::string("5"));
    std::string(std::string("5"));

    std::string s3(5, ' ');
    std::string(5, ' ');
    (void)std::string(5, ' ');
    std::string s4(std::string(5, ' '));
    std::string(std::string(5, ' '));

    static volatile int i;
    if (i) return std::string();
    if (i) return std::string("5");
    if (i) return std::string(5, ' ');

    for (std::string(); i; std::string()) {
    }

    return std::string();
}

}
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
