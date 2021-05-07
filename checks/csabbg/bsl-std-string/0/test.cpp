#include <bsl_string.h>
#include <bsl_string_view.h>

using namespace BloombergLP;

int main() {
    bsl::string_view hello = "hello";
    bsl::string s(hello);
    return 0;
}
