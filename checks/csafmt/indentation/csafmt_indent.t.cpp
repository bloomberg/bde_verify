// csafmt_indent.t.cpp                                                -*-C++-*-
#include <assert.h>

#define F() \
    do { \
        extern volatile int x; \
        ++x; \
    } while (false);

#define G()

int f()
    {
  F(); F();
      assert(
        2
      );
        G();
    F(
  );
        assert(7);
        return 0;
}

void g(int a, int b,
       int c)
{
}

void h(int a,
       int b, int c)
{
}

void i(int a, int b, int c)
{
}

void j(int a,
       int b,
      int c)
{
}

void abcdefghij(int);
int klmnopqrst(int, int, int);
void k()
{
    abcdefghij(klmnopqrst(193,
                          52,
                          -111111111));
    (&abcdefghij)((&klmnopqrst)(193,
                                52,
                                -111111111));
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
