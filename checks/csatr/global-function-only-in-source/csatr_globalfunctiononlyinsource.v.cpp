// csatr_globalfunctiononlyinsource.v.cpp                             -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

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
