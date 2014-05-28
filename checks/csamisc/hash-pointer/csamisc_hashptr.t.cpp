// csamisc_hashptr.t.cpp                                              -*-C++-*-
#if __cplusplus >= 201103

#include <functional>

namespace bde_verify {
namespace {

void f()
{
    std::hash<                        char    *>()( "abc");
    std::hash<const                   char    *>()( "abc");
    std::hash<                        wchar_t *>()(L"abc");
    std::hash<const                   wchar_t *>()(L"abc");
    std::hash<const                   void    *>()( "abc");
    double d;
    std::hash<double *>()(&d);
}

}
}

#endif  // __cplusplus >= 201103
