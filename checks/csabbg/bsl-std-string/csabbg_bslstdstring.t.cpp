#undef BSL_OVERRIDES_STD

#include <bsl_string.h>
#include <string>

static void f(bsl::string) { }
static void g(std::string) { }

int main()
{
    std::string s;
    bsl::string b;

    s = b;
    b = s;

    f(s);
    g(b);

    std::string x(b);
    bsl::string y(s);
}
