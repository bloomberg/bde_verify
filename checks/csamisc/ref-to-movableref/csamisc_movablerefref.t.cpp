// csamisc_movablerefref.t.cpp                                        -*-C++-*-

#include <string>

namespace bde_verify {

namespace bslmf { template <class T> struct MovableRef { }; }
namespace bslmf { template <class T> struct X { }; }

void f(bslmf::MovableRef<int> a,
       const bslmf::MovableRef<int> b,
       bslmf::MovableRef<int> *c,
       const bslmf::MovableRef<int> *d,
       bslmf::MovableRef<int> &e,
       const bslmf::MovableRef<int> &f,
       const int &x);

template <class T>
void g(bslmf::MovableRef<T> a,
       const bslmf::MovableRef<T> b,
       bslmf::MovableRef<T> *c,
       const bslmf::MovableRef<T> *d,
       bslmf::MovableRef<T> &e,
       const bslmf::MovableRef<T> &f,
       const T &x);

void h(bslmf::X<int> a,
       const bslmf::X<int> b,
       bslmf::X<int> *c,
       const bslmf::X<int> *d,
       bslmf::X<int> &e,
       const bslmf::X<int> &f,
       const int &x);

template <class T>
void i(bslmf::X<T> a,
       const bslmf::X<T> b,
       bslmf::X<T> *c,
       const bslmf::X<T> *d,
       bslmf::X<T> &e,
       const bslmf::X<T> &f,
       const T &x);

namespace bslmf {

void j(MovableRef<int> a,
       const MovableRef<int> b,
       MovableRef<int> *c,
       const MovableRef<int> *d,
       MovableRef<int> &e,
       const MovableRef<int> &f,
       const int &x);

template <class T>
void k(MovableRef<T> a,
       const MovableRef<T> b,
       MovableRef<T> *c,
       const MovableRef<T> *d,
       MovableRef<T> &e,
       const MovableRef<T> &f,
       const T &x);

}

void r()
{
    auto s = &g<int>;
    auto t = &i<int>;
    auto u = &g<char>;
    auto v = &i<char>;
    auto w = &bslmf::k<int>;
    auto x = &bslmf::k<char>;
}

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
// Ibslmf::MPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEbslmf::MENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIbslmf::M, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FRObslmf::M, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
