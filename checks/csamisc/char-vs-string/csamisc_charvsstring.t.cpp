// csamisc_charvsstring.t.cpp                                         -*-C++-*-

namespace bde_verify
{
    namespace csamisc
    {
        void f(const char*);
        void f(int, const char*);
        void f(int, const char*, int);

        namespace {
            struct g
            {
                g(const char*) {}                                   // IMPLICIT
                g(int, const char*) {}
                void operator()(const char*) {}
                void operator()(int, const char*) {}
            };
        }
    }
}

using namespace bde_verify::csamisc;

int main()
{
    char array[] = "hello";
    char value('/');
    f("hello"); // OK
    f(array); // OK
    f(&array[0]); // OK
    f(&value); // not OK
    f(0, "hello"); // OK
    f(0, array); // OK
    f(0, &array[0]); // OK
    f(0, &value); // not OK
    f(0, "hello", 1); // OK
    f(0, array, 1); // OK
    f(0, &array[0], 1); // OK
    f(0, &value, 1); // not OK

    g g0("hello"); // OK
    g g1(array); // OK
    g g2(&array[0]); // OK
    g g3(&value); // not OK
    g g4(0, "hello"); // OK
    g g5(0, array); // OK
    g g6(0, &array[0]); // OK
    g g7(0, &value); // not OK

    g0("hello"); // OK
    g1(array); // OK
    g2(&array[0]); // OK
    g3(&value); // not OK
    g4(0, "hello"); // OK
    g5(0, array); // OK
    g6(0, &array[0]); // OK
    g7(0, &value); // not OK
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
