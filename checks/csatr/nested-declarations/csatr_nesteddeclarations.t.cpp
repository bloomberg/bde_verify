// csatr_nesteddeclarations.t.cpp                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "csatr_nesteddeclarations.t.hpp"
#include <bdes_ident.h>

// -----------------------------------------------------------------------------

struct bad_global_struct1;
struct bad_global_struct2 {};
int bad_global = 0;
void bad_global_function() {}

namespace
{
    struct bad_anonymous_struct1;
    struct bad_anonymous_struct2 {};
    int bad_anonymous_global = 0;
    void bad_anonymous_function() {}
}

namespace csatr
{
    struct bad_csatr_struct1;
    struct bad_csatr_struct2 {};
    int bad_csatr_variable = 0;
    void bad_csatr_function() {}

    namespace
    {
        struct bad_csatr_anonymous_struct1;
        struct bad_csatr_anonymous_struct2 {};
        int bad_csatr_anonymous_variable = 0;
        void bad_csatr_anonymous_function() {}
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
            struct bad_top_csatr_anonymous_struct1;
            struct bad_top_csatr_anonymous_struct2 {};
            int bad_top_csatr_anonymous_variable = 0;
            void bad_top_csatr_anonymous_function() {}
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
