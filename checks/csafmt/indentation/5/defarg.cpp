#include <bslma_default.h>
#include <bsl_string.h>

using namespace BloombergLP;

bsl::string& hostMacroMap()
{
    static bsl::string s_hostMacroMap(bslma::Default::globalAllocator());
    return s_hostMacroMap;
}
