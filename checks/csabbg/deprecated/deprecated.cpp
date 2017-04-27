#include <dep1.h>
#include <dep2.h>
void lf();
    // !DEPRECATED!:
struct LE {
    static void k();
        // !DEPRECATED!:
    static void l();
};
template <class PP>
struct LF {
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
struct LEE {
        // !DEPRECATED!:
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
template <class PP>
struct LFF {
        // !DEPRECATED!:
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
void g() {
    f();
    h();
    i();
    C::j();
    C::k();
    C::l();
    D<int>::j();
    D<void>::k();
    D<C>::l();
    E::j();
    E::k();
    E::l();
    F<int>::j();
    F<void>::k();
    F<C>::l();
    CC::j();
    CC::k();
    CC::l();
    DD<int>::j();
    DD<void>::k();
    DD<C>::l();
    EE::j();
    EE::k();
    EE::l();
    FF<int>::j();
    FF<void>::k();
    FF<C>::l();
    lf();
    LE::k();
    LE::l();
    LF<int>::j();
    LF<void>::k();
    LF<C>::l();
    LEE::j();
    LEE::k();
    LEE::l();
    LFF<int>::j();
    LFF<void>::k();
    LFF<C>::l();
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
