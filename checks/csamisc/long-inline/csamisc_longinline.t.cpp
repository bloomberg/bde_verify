// csamisc_longinline.t.cpp                                           -*-C++-*-

namespace bde_verify
{
    namespace csamisc
    {
        inline
        void f()
        { ; //  1
        }   //  2

        inline
        void g()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
        }     //  9

        inline
        void h()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
        }     // 10

        inline
        void i()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
        }     // 11

        inline
        void j()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
            ; // 11
        }     // 12

        inline
        void k()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
            ; // 11
            ; // 12
        }     // 13
    }
}

// -----------------------------------------------------------------------------

int main()
{
        // this space is intentionally left empty
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
