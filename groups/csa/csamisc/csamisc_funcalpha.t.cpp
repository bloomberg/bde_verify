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
    }
}
