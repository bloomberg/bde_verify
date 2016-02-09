#include <deprecated.h>
void f();
    // !DEPRECATED!:
struct E {
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
template <class PP>
struct F {
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
struct EE {
        // !DEPRECATED!:
    static void j();
    static void k();
        // !DEPRECATED!:
    static void l();
};
template <class PP>
struct FF {
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
}

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
