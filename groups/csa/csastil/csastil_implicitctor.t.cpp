// csastil_implicitctor.t.cpp                                         -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

namespace bde_verify {
    namespace csastil {
        namespace
        {
            struct foo {
                foo(foo const&); // -> no warning: this isn't really a conversion
                //foo(foo&&); // -> no warning: this isn't really a conversion
                foo(); // -> no warning: not a conversion
                foo(bool, bool); // -> no warning: not a conversion
                explicit foo(bool); // -> no warning: implicit
                foo(int); // -> warn
                foo(double);                                        // IMPLICIT
                foo(long double);
                                                                    // IMPLICIT
                foo(char = ' '); // -> warn: this is still qualifying for implicit conversions
                foo(short, bool = true, bool = false); // -> warn: ... as is this
                foo(long) {} // -> warn: it is also a definition ...
                foo(unsigned char) {}                               // IMPLICIT
            };

            template <class T>
            struct bar
            {
                bar(int); // -> warn
                bar(char);                                          // IMPLICIT
            };

            template <class T>
            bar<T>::bar(int)
            {
            }

            template <class T>
            bar<T>::bar(char)
            {
            }
        }
    }
}

// ... and none of these should warn:

bde_verify::csastil::foo::foo(bde_verify::csastil::foo const&) {}
//foo::foo(foo&&) {}
bde_verify::csastil::foo::foo(int) {}
bde_verify::csastil::foo::foo(bool) {}
bde_verify::csastil::foo::foo(double) {}
bde_verify::csastil::foo::foo(char) {}
bde_verify::csastil::foo::foo(short, bool, bool) {}

int main()
{
    bde_verify::csastil::bar<int> b0(0);
    bde_verify::csastil::bar<int> b1('c');
}
