// parms.cpp                                                          -*-C++-*-
void OPER_DO_ALIGNED_WORD(unsigned long long *,      unsigned long long     );
void OPER_DO_BITS(        unsigned long long *, int, unsigned long long, int);

template <
 void OPER_DO_ALIGNED_WORD(unsigned long long *,      unsigned long long     ),
 void OPER_DO_BITS(        unsigned long long *, int, unsigned long long, int)
>
struct x;

void f(
 void OPER_DO_ALIGNED_WORD(unsigned long long *,      unsigned long long     ),
 void OPER_DO_BITS(        unsigned long long *, int, unsigned long long, int)
);

template <int A, int B,
          class C>
struct y;

template <int A,
          unsigned B>
struct z;

template <
    int A,
    unsigned B
>
struct w;

void f(int A, int B,
          class C);

void f(int A,
          unsigned B);

void f(
    int A,
    unsigned B
);

void f(int A,
    unsigned B
);

template <class A,
          int   B>
struct y<B, B, A> { };

template <class A,
          class B>
struct X {
    template <class C,
              class D>
    struct Y {
        template <class E,
                  class F>
        struct Z;
        template <class E,
                  class F>
        void f();
    };
};

template <class A,
          class B>
template <class C,
          class D>
template <class E,
          class F>
struct X<A,B>::Y<C,D>::Z {
};

template <class A,
          class B>
template <class C,
          class D>
template <class E,
          class F>
void X<A,B>::Y<C,D>::f() {
};

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
