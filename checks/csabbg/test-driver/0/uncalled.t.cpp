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
// [ 1] joe();
// [ 1] joe(int uncalled);
// [ 1] void f1();
// [ 1] void f1(int uncalled);
// [ 1] void f2();
// [ 1] void f2(int uncalled);
// [ 1] void f3();
// [ 1] void f3(int uncalled);
// [ 1] static void f4();
// [ 1] static void f4(int uncalled);
// [ 1] static void f5();
// [ 1] static void f5(int uncalled);
// [ 1] static void f6();
// [ 1] static void f6(int uncalled);
// [ 1] static void f7();
// [ 1] static void f7(int uncalled);
// [ 1] void f8(int uncalled);
// [ 1] static void f9(int uncalled);
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
//    joe : just a class

namespace bde_verify
{
    struct joe {
        joe();
        joe(int uncalled);
        void f1();
        void f1(int uncalled);
        void f2();
        void f2(int uncalled);
        void f3();
        void f3(int uncalled);
        static void f4();
        static void f4(int uncalled);
        static void f5();
        static void f5(int uncalled);
        static void f6();
        static void f6(int uncalled);
        static void f7();
        static void f7(int uncalled);
        void f8(int uncalled);
        static void f9(int uncalled);
    };
    void f8(int);
    void f9(int);
}

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
        // DETECT UNCALLED METHODS
        //
        // Concerns:
        //:  1 Bde_verify detects uncalled methods.
        //
        // Plan:
        //:  1 Don't call some methods.
        //:  2 See whether bde_verify notices.
        //:  3 ???
        //:  4 Profit!
        //
        // Testing:
        //   joe();
        //   joe(int uncalled);
        //   void f1();
        //   void f1(int uncalled);
        //   void f2();
        //   void f2(int uncalled);
        //   void f3();
        //   void f3(int uncalled);
        //   static void f4();
        //   static void f4(int uncalled);
        //   static void f5();
        //   static void f5(int uncalled);
        //   static void f6();
        //   static void f6(int uncalled);
        //   static void f7();
        //   static void f7(int uncalled);
        //   void f8(int uncalled);
        //   static void f9(int uncalled);
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "DETECT UNCALLED METHODS" << endl
                          << "=======================" << endl;

          joe j, &rj = j, *pj = &j;

          j.f1();
          rj.f2();
          pj->f3();
          joe::f4();
          j.f5();
          rj.f6();
          pj->f7();
          f8(0);
          f9(0);
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
