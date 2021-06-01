#include <bsl_string.h>
#include <bslstl_stringref.h>

using namespace BloombergLP;

int main() {
    bslstl::StringRef hello = "hello";
    bsl::string       s("helllo");
    if (s == hello) {
        return 1;
    }
    return 0;
}
