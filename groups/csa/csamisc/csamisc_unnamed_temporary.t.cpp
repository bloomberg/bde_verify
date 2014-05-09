// csamisc_unnamed_temporary.t.cpp                                    -*-C++-*-

#include <string>

namespace bde_verify {
namespace {

std::string f()
{
    int i1(5);
    int(5);
    (void)int(5);
    int i2(int(5));

    std::string s1("5");
    std::string("5");
    (void)std::string("5");
    std::string s2(std::string("5"));
    std::string(std::string("5"));

    std::string s3(5, ' ');
    std::string(5, ' ');
    (void)std::string(5, ' ');
    std::string s4(std::string(5, ' '));
    std::string(std::string(5, ' '));

    static volatile int i;
    if (i) return std::string();
    if (i) return std::string("5");
    if (i) return std::string(5, ' ');

    for (std::string(); i; std::string()) {
    }

    return std::string();
}

}
}
