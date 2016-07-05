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

#define ASSERT       BSLIM_TESTUTIL_ASSERT
#define ASSERTV      BSLIM_TESTUTIL_ASSERTV

#define LOOP_ASSERT  BSLIM_TESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLIM_TESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLIM_TESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLIM_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLIM_TESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLIM_TESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLIM_TESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLIM_TESTUTIL_LOOP6_ASSERT

#define Q            BSLIM_TESTUTIL_Q   // Quote identifier literally.
#define P            BSLIM_TESTUTIL_P   // Print identifier and value.
#define P_           BSLIM_TESTUTIL_P_  // P(X) without '\n'.
#define T_           BSLIM_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BSLIM_TESTUTIL_L_  // current Line number

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
