// csastil_leakingmacro.t.cpp                                         -*-C++-*-

#include "csastil_leakingmacro.t.h"
#include <bdes_ident.h>

#define FOO 0

int main(int ac, char*[])
{
    return ac? FOO: 1;
}
