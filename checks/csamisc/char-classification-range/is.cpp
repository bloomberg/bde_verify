#include <cctype>
#include <ctype.h>

struct X { int isspace(int); int f() { return isspace(1000); } };
struct Y {                   int f() { return isspace(1000); } };
template <class T>
struct V { int isspace(int); int f() { return isspace(1000); } };
template <class T>
struct W {                   int f() { return isspace(1000); } };

int main(int c , char **v)
{
    return std::isspace(v[0][0]) +
           std::isspace(1000000) +
           std::isspace('a') +
           std::isspace(0x80) +
           std::isspace(255) +
           std::isspace(256) +
           std::isspace('\x80') +
           std::isspace('\xFF') +
           std::isspace(-1) +
           std::isspace(-2) +
           std::isspace(char(-1)) +
           isspace(v[0][0]) +
           isspace(1000000) +
           isspace('a') +
           isspace(0x80) +
           isspace(255) +
           isspace(256) +
           isspace('\x80') +
           isspace('\xFF') +
           isspace(-1) +
           isspace(-2) +
           isspace(char(-1));
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
