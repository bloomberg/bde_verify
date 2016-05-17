// csatr_globalfunctiononlyinsource.v.cpp                             -*-C++-*-

#include "csatr_globalfunctiononlyinsource.v.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csatr
    {
        void operator+(bar&) {}
        void in_namespace();
        namespace
        {
            void in_unnamed_namespace()
            {
            }
        }

        namespace
        {
            struct bar
            {
                void member();
            };
        }

// ----------------------------------------------------------------------------

        namespace
        {
            template <class> struct local;
            template <>
            struct local<bde_verify::csatr::bar>
            {
                static void member();
            };
            void local<bde_verify::csatr::bar>::member()
            {
            }
        }
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
