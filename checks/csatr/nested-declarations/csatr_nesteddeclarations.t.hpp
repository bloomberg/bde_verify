// csatr_nesteddeclarations.t.hpp                                     -*-C++-*-

#ifndef INCLUDED_CSATR_NESTEDDECLARATIONS
#define INCLUDED_CSATR_NESTEDDECLARATIONS

#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

#ifndef INCLUDED_CSATR_OTHERS
#include "csatr_others.hpp"
#endif

// -----------------------------------------------------------------------------

struct good_global_struct;
struct nesteddeclarations_bad_global_struct
{
    int member;
    void method();
    struct nested {};
};
void nesteddeclarations_bad_global_function();
int nesteddeclarations_bad_global_variable;

namespace bde_verify
{
    struct good_bde_verify_struct;
    struct nesteddeclarations_bad_bde_verify_struct
    {
        int member;
        void method();
        struct nested {};
    };
    void nesteddeclarations_bad_bde_verify_function();
    int nesteddeclarations_bad_bde_verify_variable;

    namespace csamisc
    {
        struct good_bde_verify_csamisc_struct;
        struct nesteddeclarations_bad_bde_verify_csamisc_struct
        {
            int member;
            void method();
            struct nested {};
        };
        void nesteddeclarations_bad_bde_verify_csamisc_function();
        int nesteddeclarations_bad_bde_verify_csamisc_variable;
    }

    namespace csatr
    {
        struct good_struct1;
        struct nesteddeclarations_good_struct2
        {
            struct good_nested {};
            void good_method();
            int  good_member;
        };

        void nesteddeclarations_good_function();
        int nesteddeclarations_good_bde_verify_csatr_variable;
    }

    struct csatr_NestedDeclarations
    {
    };

    void swap(csatr_NestedDeclarations&, csatr_NestedDeclarations&);
    bool operator== (const csatr_NestedDeclarations&,
                    const  csatr_NestedDeclarations&);
    bool operator!= (const csatr_NestedDeclarations&,
                    const  csatr_NestedDeclarations&);
}

// -----------------------------------------------------------------------------

#endif

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
