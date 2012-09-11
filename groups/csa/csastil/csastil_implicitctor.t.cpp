// csastil_implicitctor.t.cpp                                         -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

namespace cool {
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

cool::csastil::foo::foo(cool::csastil::foo const&) {}
//foo::foo(foo&&) {}
cool::csastil::foo::foo(int) {}
cool::csastil::foo::foo(bool) {}
cool::csastil::foo::foo(double) {}
cool::csastil::foo::foo(char) {}
cool::csastil::foo::foo(short, bool, bool) {}

int main()
{
    cool::csastil::bar<int> b0(0);
    cool::csastil::bar<int> b1('c');
}
