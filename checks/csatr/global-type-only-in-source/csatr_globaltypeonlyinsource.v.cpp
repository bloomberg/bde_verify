// csatr_globaltypeonlyinsource.v.cpp                                 -*-C++-*-

#include "csatr_globaltypeonlyinsource.v.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csatr
    {
                         struct          { int member; } s_val;
                 typedef struct          { int member; } s_typedef;
                         struct s_extern { int member; };
        namespace      { struct s_local  { int member; }; }

                         class           { int member; } c_val;
                 typedef class           { int member; } c_typedef;
                         class  c_extern { int member; };
        namespace      { class  c_local  { int member; }; }

                         enum            { e0_member   } e_val;
                 typedef enum            { e1_member   } e_typedef;
                         enum   e_extern { e2_member   };
        namespace      { enum   e_local  { e3_member   }; }

                         typedef int t_typedef;
        namespace      { typedef int t_typedef; }
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
