// csastil_implicitctor.t.cpp                                         -*-C++-*-

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
                bar(double);  // IMPLICIT
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
