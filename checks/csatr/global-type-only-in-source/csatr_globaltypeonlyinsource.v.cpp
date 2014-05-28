// csatr_globaltypeonlyinsource.v.cpp                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

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
