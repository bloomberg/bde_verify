#include <stdlib.h>
#include <bsl_iostream.h>

namespace bde_verify { }
using namespace bde_verify;
using namespace bsl;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// Nothing.
//-----------------------------------------------------------------------------
// [ 1] void joe<T>::f1();
// [ 1] void joe<T>::f2();
// [ 1] void joe<T>::f3();
// [ 1] void joe<T>::g1();
// [ 1] void joe<T>::g2();
// [ 1] void joe<T>::g3();
// [ 1] void moe::i1();
// [ 1] void moe::i2();
// [ 1] void moe::i3();
// [ 1] void moe::j1();
// [ 1] void moe::j2();
// [ 1] void moe::j3();
//-----------------------------------------------------------------------------

// ============================================================================
//                     STANDARD BSL ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", line, message);

        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BDE TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT       BDLS_TESTUTIL_ASSERT
#define ASSERTV      BDLS_TESTUTIL_ASSERTV

#define LOOP_ASSERT  BDLS_TESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BDLS_TESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BDLS_TESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BDLS_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BDLS_TESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BDLS_TESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BDLS_TESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BDLS_TESTUTIL_LOOP6_ASSERT

#define Q            BDLS_TESTUTIL_Q   // Quote identifier literally.
#define P            BDLS_TESTUTIL_P   // Print identifier and value.
#define P_           BDLS_TESTUTIL_P_  // P(X) without '\n'.
#define T_           BDLS_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BDLS_TESTUTIL_L_  // current Line number

//@CLASSES:
//    joe : just a class template
//    moe : just a class

namespace bde_verify
{
    template <class T> struct joe {
        void f1();
        void f2();
        void f3();
        static void g1();
        static void g2();
        static void g3();
    };

    struct moe {
        void i1();
        void i2();
        void i3();
        static void j1();
        static void j2();
        static void j3();
    };
}

static void (joe<void>::*ejpf)() = &joe<void>::f3;
static void (moe::*empf)() = &moe::i3;
static void (*ejspf)() = &joe<void>::g3;
static void (*emspf)() = &moe::j3;

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test     = argc > 1 ? atoi(argv[1]) : 0;
    bool verbose = argc > 2;

    switch (test) {
      case 1: {
        // --------------------------------------------------------------------
        // CALLS THROUGH POINTERS
        //
        // Concerns:
        //:  1 Bde_verify detects calls through pointers.
        //
        // Plan:
        //:  1 Store pointers to methods.
        //
        // Testing:
        //   void joe<T>::f1();
        //   void joe<T>::f2();
        //   void joe<T>::f3();
        //   void joe<T>::g1();
        //   void joe<T>::g2();
        //   void joe<T>::g3();
        //   void moe::i1();
        //   void moe::i2();
        //   void moe::i3();
        //   void moe::j1();
        //   void moe::j2();
        //   void moe::j3();
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "CALLS THROUGH POINTERS" << endl
                          << "======================" << endl;

          joe<void> j;
          moe       m;

          void (joe<void>::*jpf)() = &joe<void>::f1;
          void (joe<void>::*jpfa[])() = { &joe<void>::f1, &joe<void>::f2 };
          void (moe::*mpf)() = &moe::i1;
          void (moe::*mpfa[])() = { &moe::i1, &moe::i2 };
          void (*jspf)() = &joe<void>::g1;
          void (*(jspfa[]))() = { &joe<void>::g1, &joe<void>::g2 };
          void (*mspf)() = &moe::j1;
          void (*(mspfa[]))() = { &moe::j1, &moe::j2 };
      } break;
    }
    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
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
