// csatr_includefiles.t.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------

#include "csatr_includefiles.t.h"

#if 0
namespace bde_verify
{
    namespace csatr
    {
        namespace
        {
            class local_class {};
        }

        namespace
        {
            enum E { eval };
            class C { };
            class D;
            typedef D F;
            typedef C G;

            template <typename T, int S> struct Templ {};

            extern int var_builtin;
            extern E   var_enum;
            extern C   var_class;
            extern G   var_typedef;

            extern int* ptr_builtin;
            extern E*   ptr_enum;
            extern C*   ptr_class;
            extern F*   ptr_typedef;

            extern int& ref_builtin;
            extern E&   ref_enum;
            extern C&   ref_class;
            extern F&   ref_typedef;

            extern int D::*member_builtin;
            extern E   D::*member_enum;
            extern C   D::*member_class;
            extern F   D::*member_typedef;

            extern int array_builtin[3];
            extern E   array_enum[3];
            extern C   array_class[3];
            extern G   array_typedef[3];

            extern int incomplete_array_builtin[];
            extern E   incomplete_array_enum[];
            extern C   incomplete_array_class[];
            extern G   incomplete_array_typedef[];

            extern Templ<int, 3> templ_builtin;
            extern Templ<E, 3>   templ_enum;
            extern Templ<C, 3>   templ_class;
            extern Templ<G, 3>   templ_typedef;

            int var_builtin(0);
            E   var_enum(eval);
            C   var_class;
            G   var_typedef;

            int* ptr_builtin(0);
            E* ptr_enum(0);
            C* ptr_class(0);
            F* ptr_typedef(0);

            int& ref_builtin(*ptr_builtin);
            E&   ref_enum(*ptr_enum);
            C&   ref_class(*ptr_class);
            F&   ref_typedef(*ptr_typedef);

            int D::*member_builtin;
            E   D::*member_enum;
            C   D::*member_class;
            F   D::*member_typedef;

            int array_builtin[3];
            E   array_enum[3];
            C   array_class[3];
            G   array_typedef[3];

            Templ<int, 3> templ_builtin;
            Templ<E, 3>   templ_enum;
            Templ<C, 3>   templ_class;
            Templ<G, 3>   templ_typedef;

        }
    }
}
#endif

int                             bde_verify::csatr::includeFilesVarBuiltin;
bde_verify::csatr::IncludeVarEnum    bde_verify::csatr::includeFilesVarEnum;
bde_verify::csatr::IncludeVarClass   bde_verify::csatr::includeFilesVarClass;
bde_verify::csatr::IncludeVarTypedef bde_verify::csatr::includeFilesVarTypedef;

int main(int ac, char*[])
{
    //bde_verify::csatr::local_class();
    return ac;
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
