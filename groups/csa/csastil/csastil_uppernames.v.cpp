// csastil_uppernames.t.cpp                                           -*-C++-*-

#include <stdio.h>

namespace bde_verify {
namespace csastil {
namespace {

int AA;

void BB()
{
    int BB_AA;
    struct BB_CC
    {
    };
}

struct CC
{
    int CC_AA;
    template <class CC_DD> struct CC_EE { };
};

}

template <class DD> struct EE
{
    int EE_AA;
    struct EE_CC  { };
    template <class EE_DD> struct EE_EE { };
    template <class EE_FF> struct EE_GG;
};

}
}

template <class EE_HH>
template <class EE_II>
struct bde_verify::csastil::EE<EE_HH>::EE_GG
{
    EE_HH JJ;
    EE_II KK;
};

int LL_CSASTIL_UPPERNAMES;
