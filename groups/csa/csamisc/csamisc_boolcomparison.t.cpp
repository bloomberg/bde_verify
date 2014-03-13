// csamisc_boolcomparison.t.cpp                                       -*-C++-*-

namespace bde_verify
{
    namespace csamisc
    {
        bool boolcomparison();
    }
}

int main()
{
    typedef bool Boolean;
    bool       b0 = bde_verify::csamisc::boolcomparison();
    Boolean    b1 = bde_verify::csamisc::boolcomparison();
    bool const b2 = bde_verify::csamisc::boolcomparison();
    int  i(0);

    if (i     == 0) {}
    if (i     == true) {}
    if (i     == false) {}
    if (b0    == 0) {}
    if (b0    == true) {}
    if (b0    == false) {}
    if (b1    == 0) {}
    if (b1    == true) {}
    if (b1    == false) {}
    if (b2    == 0) {}
    if (b2    == true) {}
    if (b2    == false) {}
    if (0     == i) {}
    if (true  == i) {}
    if (false == i) {}
    if (0     == b0) {}
    if (true  == b0) {}
    if (false == b0) {}
    if (0     == b1) {}
    if (true  == b1) {}
    if (false == b1) {}
    if (0     == b2) {}
    if (true  == b2) {}
    if (false == b2) {}

    if (true  != b0) {}
    if (true  <= b0) {}
    if (true  <  b0) {}
    if (true  >= b0) {}
    if (true  >  b0) {}
    if (true  |  b0) {}
    if (true  +  b0) {}
    if (true  || b0) {}
}
