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
