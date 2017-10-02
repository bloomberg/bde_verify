// csamisc_funcalpha.t.cpp                                            -*-C++-*-

namespace bde_verify {
    namespace {
        class X {
            void f1();
            void f3();
            void f2();

            // - - - -

            void a1();
            void a2();
            void a3();

            // = = = =

            void a9();
            void a10();

            // - - - -

            void a100();
            void a99();
        };

        void X::f3() { }
        void X::f2() { }
        void f1() { }

        // - - - -

        void z();
        void y() { }
        void x();

        // - - - -

        void zz() { }
        void y();
        void x() { }
    }
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
