#include <dept.h>
template <class T> void f();
    // !DEPRECATED!:
struct E {
    template <class T> static void j();
    template <class T> static void k();
        // !DEPRECATED!:
    template <class T> static void l();
};
template <class PP>
struct F {
    template <class T> static void j();
    template <class T> static void k();
        // !DEPRECATED!:
    template <class T> static void l();
};
struct EE {
        // !DEPRECATED!:
    template <class T> static void j();
    template <class T> static void k();
        // !DEPRECATED!:
    template <class T> static void l();
};
template <class PP>
struct FF {
        // !DEPRECATED!:
    template <class T> static void j();
    template <class T> static void k();
        // !DEPRECATED!:
    template <class T> static void l();
};
void g() {
    f<int>();
    h<int>();
    i<int>();
    C::j<int>();
    C::k<int>();
    C::l<int>();
    D<int>::j<int>();
    D<void>::k<int>();
    D<C>::l<int>();
    E::j<int>();
    E::k<int>();
    E::l<int>();
    F<int>::j<int>();
    F<void>::k<int>();
    F<C>::l<int>();
    CC::j<int>();
    CC::k<int>();
    CC::l<int>();
    DD<int>::j<int>();
    DD<void>::k<int>();
    DD<C>::l<int>();
    EE::j<int>();
    EE::k<int>();
    EE::l<int>();
    FF<int>::j<int>();
    FF<void>::k<int>();
    FF<C>::l<int>();
}

// ----------------------------------------------------------------------------
// Copyright (C) 2016 Bloomberg Finance L.P.
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
