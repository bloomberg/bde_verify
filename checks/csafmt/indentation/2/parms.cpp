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
