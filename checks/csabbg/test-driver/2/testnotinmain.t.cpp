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
// [ 1] void joe<T>::f();
// [ 1] void joe<T>::g();
// [ 1] void joe<T>::h();
// [ 1] void moe::i();
// [ 1] void moe::j();
// [ 1] void moe::k();
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
        void f();
        void g();
        void h();
    };

    struct moe {
        void i();
        void j();
        void k();
    };
}

#include <notinmain.h>

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

template <class T>
void call_g(joe<T> &j) { j.g(); }
void call_j(moe    &j) { j.j(); }

int main(int argc, char *argv[])
{
    int test     = argc > 1 ? atoi(argv[1]) : 0;
    bool verbose = argc > 2;

    switch (test) {
      case 1: {
        // --------------------------------------------------------------------
        // CALLS OUTSIDE OF MAIN
        //
        // Concerns:
        //:  1 Bde_verify detects calls outside of main but in test driver.
        //
        // Plan:
        //:  1 Call one method in main.
        //:  2 Call a function defined in this file that calls another method.
        //:  3 Call a function in a header that calls another method.
        //
        // Testing:
        //   void joe<T>::f();
        //   void joe<T>::g();
        //   void joe<T>::h();
        //   void moe::i();
        //   void moe::j();
        //   void moe::k();
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "CALLS OUTSIDE OF MAIN" << endl
                          << "=====================" << endl;

          joe<void> j;
          moe       m;
          j.f();
          call_g(j);
          call_h(j);
          m.i();
          call_j(m);
          call_k(m);
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
