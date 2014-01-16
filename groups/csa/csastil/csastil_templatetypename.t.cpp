// csastil_templatetypename.t.cpp                                     -*-C++-*-

#include "csastil_templatetypename.t.h"
#include <bdes_ident.h>

template <typename T>
static void bad();

template <typename TT>
static void good();

namespace cool
{
    namespace csastil
    {
        namespace
        {
            template <typename T>
            static void localBad();

            template <class TT>
            static void localGood();
        }
    }
}
