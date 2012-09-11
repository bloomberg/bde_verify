// csastil_templatetypename.t.cpp                                     -*-C++-*-

#include "csastil_templatetypename.t.h"

template <typename T>
static void bad();

template <typename T>
static void good();

namespace cool
{
    namespace csastil
    {
        namespace
        {
            template <typename T>
            static void localBad();

            template <class T>
            static void localGood();
        }
    }
}
