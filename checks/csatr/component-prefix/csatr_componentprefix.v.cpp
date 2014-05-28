// csatr_componentprefix.v.cpp                                        -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include "csatr_componentprefix.v.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csatr
    {
        namespace
        {
            struct GoodStruct
            {
            };

            class GoodClass
            {
            };

            template <class>
            class GoodTemplate
            {
            };

            void goodFunction()
            {
            }

            template <class>
            void goodTemplate()
            {
            }

            int goodVariable(0);
        }
    }
}
