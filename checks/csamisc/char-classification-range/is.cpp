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
