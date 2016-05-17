// csatr_nesteddeclarations.t.cpp                                     -*-C++-*-

#include "csatr_nesteddeclarations.t.hpp"
#include <bdes_ident.h>

// -----------------------------------------------------------------------------

struct bad_global_struct1;
struct bad_global_struct2 {};
int bad_global = 0;
void bad_global_function() {}

namespace
{
    struct bad_anonymous_struct1;     // now ok
    struct bad_anonymous_struct2 {};  // now ok
    int bad_anonymous_global = 0;     // now ok
    void bad_anonymous_function() {}  // now ok
}

namespace csatr
{
    struct bad_csatr_struct1;
    struct bad_csatr_struct2 {};
    int bad_csatr_variable = 0;
    void bad_csatr_function() {}

    namespace
    {
        struct bad_csatr_anonymous_struct1;     // now ok
        struct bad_csatr_anonymous_struct2 {};  // now ok
        int bad_csatr_anonymous_variable = 0;   // now ok
        void bad_csatr_anonymous_function() {}  // now ok
    }
}

namespace top
{
    namespace csatr
    {
        struct bad_top_csatr_struct1;
        struct bad_top_csatr_struct2 {};
        int bad_top_csatr_variable = 0;
        void bad_top_csatr_function() {}

        namespace
        {
            struct bad_top_csatr_anonymous_struct1;     // now ok
            struct bad_top_csatr_anonymous_struct2 {};  // now ok
            int bad_top_csatr_anonymous_variable = 0;   // now ok
            void bad_top_csatr_anonymous_function() {}  // now ok
        }
    }
}

namespace bde_verify
{
    namespace csatr
    {
        struct good_source_struct1;
        struct good_source_struct2 {};
        int good_source_global = 0;
        void good_source_function() {}

        namespace
        {
            struct good_source_anonymous_struct1;
            struct good_source_anonymous_struct2 {};
            int good_source_anonymous_global = 0;
            void good_source_anonymous_function() {}
        }
    }
}

int main()
{
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
